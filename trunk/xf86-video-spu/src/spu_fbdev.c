/*
 * Authors:  Alan Hourihane, <alanh@fairlite.demon.co.uk>
 *	     Michel Dänzer, <michel@tungstengraphics.com>
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <string.h>

/* all driver need this */
#include "xf86.h"
#include "xf86_OSproc.h"

#include "mipointer.h"
#include "mibstore.h"
#include "micmap.h"
#include "colormapst.h"
#include "xf86cmap.h"
#include "shadow.h"
#include "dgaproc.h"

/* for visuals */
#include "fb.h"
#ifdef USE_AFB
#include "afb.h"
#endif

#include "xf86Resources.h"
#include "xf86RAC.h"

#include "fbdevhw.h"

#include "xf86xv.h"

#ifdef XSERVER_LIBPCIACCESS
#include <pciaccess.h>
#endif

static Bool debug = 0;

#define TRACE_ENTER(str) \
    do { if (debug) ErrorF("fbdev: " str " %d\n",pScrn->scrnIndex); } while (0)
#define TRACE_EXIT(str) \
    do { if (debug) ErrorF("fbdev: " str " done\n"); } while (0)
#define TRACE(str) \
    do { if (debug) ErrorF("fbdev trace: " str "\n"); } while (0)

/* -------------------------------------------------------------------- */
/* prototypes                                                           */

static const OptionInfoRec * SPU_FBDevAvailableOptions(int chipid, int busid);
static void	SPU_FBDevIdentify(int flags);
static Bool	SPU_FBDevProbe(DriverPtr drv, int flags);
#ifdef XSERVER_LIBPCIACCESS
static Bool	SPU_FBDevPciProbe(DriverPtr drv, int entity_num,
     struct pci_device *dev, intptr_t match_data);
#endif
static Bool	SPU_FBDevPreInit(ScrnInfoPtr pScrn, int flags);
static Bool	SPU_FBDevScreenInit(int Index, ScreenPtr pScreen, int argc,
				char **argv);
static Bool	SPU_FBDevCloseScreen(int scrnIndex, ScreenPtr pScreen);
static void *	SPU_FBDevWindowLinear(ScreenPtr pScreen, CARD32 row, CARD32 offset, int mode,
				  CARD32 *size, void *closure);
static void	SPU_FBDevPointerMoved(int index, int x, int y);
static Bool	SPU_FBDevDGAInit(ScrnInfoPtr pScrn, ScreenPtr pScreen);
static Bool	SPU_FBDevDriverFunc(ScrnInfoPtr pScrn, xorgDriverFuncOp op,
				pointer ptr);


enum { SPU_FBDEV_ROTATE_NONE=0, SPU_FBDEV_ROTATE_CW=270, SPU_FBDEV_ROTATE_UD=180, SPU_FBDEV_ROTATE_CCW=90 };


/* -------------------------------------------------------------------- */

/*
 * This is intentionally screen-independent.  It indicates the binding
 * choice made in the first PreInit.
 */
static int pix24bpp = 0;

#define SPU_FBDEV_VERSION		4000
#define SPU_FBDEV_NAME		"SPU_FBDEV"
#define SPU_FBDEV_DRIVER_NAME	"spu_fbdev"

#ifdef XSERVER_LIBPCIACCESS
static const struct pci_id_match fbdev_device_match[] = {
    {
	PCI_MATCH_ANY, PCI_MATCH_ANY, PCI_MATCH_ANY, PCI_MATCH_ANY,
	0x00030000, 0x00ffffff, 0
    },

    { 0, 0, 0 },
};
#endif

_X_EXPORT DriverRec SPU_FBDEV = {
	SPU_FBDEV_VERSION,
	SPU_FBDEV_DRIVER_NAME,
#if 0
	"spu acellerated driver for linux framebuffer devices",
#endif
	SPU_FBDevIdentify,
	SPU_FBDevProbe,
	SPU_FBDevAvailableOptions,
	NULL,
	0,
	SPU_FBDevDriverFunc,

#ifdef XSERVER_LIBPCIACCESS
    fbdev_device_match,
    SPU_FBDevPciProbe
#endif
};

/* Supported "chipsets" */
static SymTabRec SPU_FBDevChipsets[] = {
    { 0, "spu_fbdev" },
#ifdef USE_AFB
    { 0, "afb" },
#endif
    {-1, NULL }
};

/* Supported options */
typedef enum {
	OPTION_SHADOW_FB,
	OPTION_ROTATE,
	OPTION_FBDEV,
	OPTION_DEBUG
} SPU_FBDevOpts;

static const OptionInfoRec SPU_FBDevOptions[] = {
	{ OPTION_SHADOW_FB,	"ShadowFB",	OPTV_BOOLEAN,	{0},	FALSE },
	{ OPTION_ROTATE,	"Rotate",	OPTV_STRING,	{0},	FALSE },
	{ OPTION_FBDEV,		"fbdev",	OPTV_STRING,	{0},	FALSE },
	{ OPTION_DEBUG,		"debug",	OPTV_BOOLEAN,	{0},	FALSE },
	{ -1,			NULL,		OPTV_NONE,	{0},	FALSE }
};

/* -------------------------------------------------------------------- */

static const char *afbSymbols[] = {
	"afbScreenInit",
	"afbCreateDefColormap",
	NULL
};

static const char *fbSymbols[] = {
	"fbScreenInit",
	"fbPictureInit",
	NULL
};

static const char *shadowSymbols[] = {
	"shadowAdd",
	"shadowInit",
	"shadowSetup",
	"shadowUpdatePacked",
	"shadowUpdatePackedWeak",
	"shadowUpdateRotatePacked",
	"shadowUpdateRotatePackedWeak",
	NULL
};

static const char *fbdevHWSymbols[] = {
	"fbdevHWInit",
	"fbdevHWProbe",
	"fbdevHWSetVideoModes",
	"fbdevHWUseBuildinMode",

	"fbdevHWGetDepth",
	"fbdevHWGetLineLength",
	"fbdevHWGetName",
	"fbdevHWGetType",
	"fbdevHWGetVidmem",
	"fbdevHWLinearOffset",
	"fbdevHWLoadPalette",
	"fbdevHWMapVidmem",
	"fbdevHWUnmapVidmem",

	/* colormap */
	"fbdevHWLoadPalette",
	"fbdevHWLoadPaletteWeak",

	/* ScrnInfo hooks */
	"fbdevHWAdjustFrameWeak",
	"fbdevHWEnterVTWeak",
	"fbdevHWLeaveVTWeak",
	"fbdevHWModeInit",
	"fbdevHWRestore",
	"fbdevHWSave",
	"fbdevHWSaveScreen",
	"fbdevHWSaveScreenWeak",
	"fbdevHWSwitchModeWeak",
	"fbdevHWValidModeWeak",

	"fbdevHWDPMSSet",
	"fbdevHWDPMSSetWeak",

	NULL
};

#ifdef XFree86LOADER

MODULESETUPPROTO(SPU_FBDevSetup);

static XF86ModuleVersionInfo SPU_FBDevVersRec =
{
	"fbdev",
	MODULEVENDORSTRING,
	MODINFOSTRING1,
	MODINFOSTRING2,
	XORG_VERSION_CURRENT,
	PACKAGE_VERSION_MAJOR, PACKAGE_VERSION_MINOR, PACKAGE_VERSION_PATCHLEVEL,
	ABI_CLASS_VIDEODRV,
	ABI_VIDEODRV_VERSION,
	NULL,
	{0,0,0,0}
};

_X_EXPORT XF86ModuleData spu_fbdevModuleData = { &SPU_FBDevVersRec, SPU_FBDevSetup, NULL };

pointer
SPU_FBDevSetup(pointer module, pointer opts, int *errmaj, int *errmin)
{
	static Bool setupDone = FALSE;

	if (!setupDone) {
		setupDone = TRUE;
		xf86AddDriver(&SPU_FBDEV, module, HaveDriverFuncs);
		LoaderRefSymLists(afbSymbols, fbSymbols,
				  shadowSymbols, fbdevHWSymbols, NULL);
		return (pointer)1;
	} else {
		if (errmaj) *errmaj = LDR_ONCEONLY;
		return NULL;
	}
}

#endif /* XFree86LOADER */

/* -------------------------------------------------------------------- */
/* our private data, and two functions to allocate/free this            */

typedef struct {
	unsigned char*			fbstart;
	unsigned char*			fbmem;
	int				fboff;
	int				lineLength;
	int				rotate;
	Bool				shadowFB;
	void				*shadow;
	CloseScreenProcPtr		CloseScreen;
	CreateScreenResourcesProcPtr	CreateScreenResources;
	void				(*PointerMoved)(int index, int x, int y);
	EntityInfoPtr			pEnt;
	/* DGA info */
	DGAModePtr			pDGAMode;
	int				nDGAMode;
	OptionInfoPtr			Options;
} SPU_FBDevRec, *SPU_FBDevPtr;

#define SPU_FBDEVPTR(p) ((SPU_FBDevPtr)((p)->driverPrivate))

static Bool
SPU_FBDevGetRec(ScrnInfoPtr pScrn)
{
	if (pScrn->driverPrivate != NULL)
		return TRUE;
	
	pScrn->driverPrivate = xnfcalloc(sizeof(SPU_FBDevRec), 1);
	return TRUE;
}

static void
SPU_FBDevFreeRec(ScrnInfoPtr pScrn)
{
	if (pScrn->driverPrivate == NULL)
		return;
	xfree(pScrn->driverPrivate);
	pScrn->driverPrivate = NULL;
}

/* -------------------------------------------------------------------- */

static const OptionInfoRec *
SPU_FBDevAvailableOptions(int chipid, int busid)
{
	return SPU_FBDevOptions;
}

static void
SPU_FBDevIdentify(int flags)
{
	xf86PrintChipsets(SPU_FBDEV_NAME, "driver for framebuffer", SPU_FBDevChipsets);
}


#ifdef XSERVER_LIBPCIACCESS
static Bool SPU_FBDevPciProbe(DriverPtr drv, int entity_num,
			  struct pci_device *dev, intptr_t match_data)
{
    ScrnInfoPtr pScrn = NULL;

    if (!xf86LoadDrvSubModule(drv, "fbdevhw"))
	return FALSE;
	    
    xf86LoaderReqSymLists(fbdevHWSymbols, NULL);

    pScrn = xf86ConfigPciEntity(NULL, 0, entity_num, NULL, NULL,
				NULL, NULL, NULL, NULL);
    if (pScrn) {
	char *device;
	GDevPtr devSection = xf86GetDevFromEntity(pScrn->entityList[0],
						  pScrn->entityInstanceList[0]);

	device = xf86FindOptionValue(devSection->options, "fbdev");
	if (fbdevHWProbe(NULL, device, NULL)) {
	    pScrn->driverVersion = SPU_FBDEV_VERSION;
	    pScrn->driverName    = SPU_FBDEV_DRIVER_NAME;
	    pScrn->name          = SPU_FBDEV_NAME;
	    pScrn->Probe         = SPU_FBDevProbe;
	    pScrn->PreInit       = SPU_FBDevPreInit;
	    pScrn->ScreenInit    = SPU_FBDevScreenInit;
	    pScrn->SwitchMode    = fbdevHWSwitchModeWeak();
	    pScrn->AdjustFrame   = fbdevHWAdjustFrameWeak();
	    pScrn->EnterVT       = fbdevHWEnterVTWeak();
	    pScrn->LeaveVT       = fbdevHWLeaveVTWeak();
	    pScrn->ValidMode     = fbdevHWValidModeWeak();

	    xf86DrvMsg(pScrn->scrnIndex, X_CONFIG,
		       "claimed PCI slot %d@%d:%d:%d\n", 
		       dev->bus, dev->domain, dev->dev, dev->func);
	    xf86DrvMsg(pScrn->scrnIndex, X_INFO,
		       "using %s\n", device ? device : "default device");
	}
	else {
	    pScrn = NULL;
	}
    }

    return (pScrn != NULL);
}
#endif


static Bool
SPU_FBDevProbe(DriverPtr drv, int flags)
{
	int i;
	ScrnInfoPtr pScrn;
       	GDevPtr *devSections;
	int numDevSections;
	int bus,device,func;
	char *dev;
	Bool foundScreen = FALSE;

	TRACE("probe start");

	/* For now, just bail out for PROBE_DETECT. */
	if (flags & PROBE_DETECT)
		return FALSE;

	if ((numDevSections = xf86MatchDevice(SPU_FBDEV_DRIVER_NAME, &devSections)) <= 0) 
	    return FALSE;
	
	if (!xf86LoadDrvSubModule(drv, "fbdevhw"))
	    return FALSE;
	    
	xf86LoaderReqSymLists(fbdevHWSymbols, NULL);
	
	for (i = 0; i < numDevSections; i++) {
	    Bool isIsa = FALSE;
#ifndef XSERVER_LIBPCIACCESS
	    Bool isPci = FALSE;
#endif

	    dev = xf86FindOptionValue(devSections[i]->options,"fbdev");
	    if (devSections[i]->busID) {
#ifndef XSERVER_LIBPCIACCESS
	        if (xf86ParsePciBusString(devSections[i]->busID,&bus,&device,
					  &func)) {
		    if (!xf86CheckPciSlot(bus,device,func))
		        continue;
		    isPci = TRUE;
		} else
#endif
		if (xf86ParseIsaBusString(devSections[i]->busID))
		    isIsa = TRUE;
		  
	    }
	    if (fbdevHWProbe(NULL,dev,NULL)) {
		pScrn = NULL;
#ifndef XSERVER_LIBPCIACCESS
		if (isPci) {
		    /* XXX what about when there's no busID set? */
		    int entity;
		    
		    entity = xf86ClaimPciSlot(bus,device,func,drv,
					      0,devSections[i],
					      TRUE);
		    pScrn = xf86ConfigPciEntity(pScrn,0,entity,
						      NULL,RES_SHARED_VGA,
						      NULL,NULL,NULL,NULL);
		    /* xf86DrvMsg() can't be called without setting these */
		    pScrn->driverName    = SPU_FBDEV_DRIVER_NAME;
		    pScrn->name          = SPU_FBDEV_NAME;
		    xf86DrvMsg(pScrn->scrnIndex, X_CONFIG,
			       "claimed PCI slot %d:%d:%d\n",bus,device,func);

		} else
#endif
		if (isIsa) {
		    int entity;
		    
		    entity = xf86ClaimIsaSlot(drv, 0,
					      devSections[i], TRUE);
		    pScrn = xf86ConfigIsaEntity(pScrn,0,entity,
						      NULL,RES_SHARED_VGA,
						      NULL,NULL,NULL,NULL);
		} else {
		   int entity;

		    entity = xf86ClaimFbSlot(drv, 0,
					      devSections[i], TRUE);
		    pScrn = xf86ConfigFbEntity(pScrn,0,entity,
					       NULL,NULL,NULL,NULL);
		   
		}
		if (pScrn) {
		    foundScreen = TRUE;
		    
		    pScrn->driverVersion = SPU_FBDEV_VERSION;
		    pScrn->driverName    = SPU_FBDEV_DRIVER_NAME;
		    pScrn->name          = SPU_FBDEV_NAME;
		    pScrn->Probe         = SPU_FBDevProbe;
		    pScrn->PreInit       = SPU_FBDevPreInit;
		    pScrn->ScreenInit    = SPU_FBDevScreenInit;
		    pScrn->SwitchMode    = fbdevHWSwitchModeWeak();
		    pScrn->AdjustFrame   = fbdevHWAdjustFrameWeak();
		    pScrn->EnterVT       = fbdevHWEnterVTWeak();
		    pScrn->LeaveVT       = fbdevHWLeaveVTWeak();
		    pScrn->ValidMode     = fbdevHWValidModeWeak();
		    
		    xf86DrvMsg(pScrn->scrnIndex, X_INFO,
			       "using %s\n", dev ? dev : "default device");
		}
	    }
	}
	xfree(devSections);
	TRACE("probe done");
	return foundScreen;
}

static Bool
SPU_FBDevPreInit(ScrnInfoPtr pScrn, int flags)
{
	SPU_FBDevPtr fPtr;
	int default_depth, fbbpp;
	const char *mod = NULL, *s;
	const char **syms = NULL;
	int type;

	if (flags & PROBE_DETECT) return FALSE;

	TRACE_ENTER("PreInit");

	/* Check the number of entities, and fail if it isn't one. */
	if (pScrn->numEntities != 1)
		return FALSE;

	pScrn->monitor = pScrn->confScreen->monitor;

	SPU_FBDevGetRec(pScrn);
	fPtr = SPU_FBDEVPTR(pScrn);

	fPtr->pEnt = xf86GetEntityInfo(pScrn->entityList[0]);

	pScrn->racMemFlags = RAC_FB | RAC_COLORMAP | RAC_CURSOR | RAC_VIEWPORT;
	/* XXX Is this right?  Can probably remove RAC_FB */
	pScrn->racIoFlags = RAC_FB | RAC_COLORMAP | RAC_CURSOR | RAC_VIEWPORT;

	if (fPtr->pEnt->location.type == BUS_PCI &&
	    xf86RegisterResources(fPtr->pEnt->index,NULL,ResExclusive)) {
		xf86DrvMsg(pScrn->scrnIndex, X_ERROR,
		   "xf86RegisterResources() found resource conflicts\n");
		return FALSE;
	}

	/* open device */
	if (!fbdevHWInit(pScrn,NULL,xf86FindOptionValue(fPtr->pEnt->device->options,"fbdev")))
		return FALSE;
	default_depth = fbdevHWGetDepth(pScrn,&fbbpp);
	if (!xf86SetDepthBpp(pScrn, default_depth, default_depth, fbbpp,
			     Support24bppFb | Support32bppFb | SupportConvert32to24 | SupportConvert24to32))
		return FALSE;
	xf86PrintDepthBpp(pScrn);

	/* Get the depth24 pixmap format */
	if (pScrn->depth == 24 && pix24bpp == 0)
		pix24bpp = xf86GetBppFromDepth(pScrn, 24);

	/* color weight */
	if (pScrn->depth > 8) {
		rgb zeros = { 0, 0, 0 };
		if (!xf86SetWeight(pScrn, zeros, zeros))
			return FALSE;
	}

	/* visual init */
	if (!xf86SetDefaultVisual(pScrn, -1))
		return FALSE;

	/* We don't currently support DirectColor at > 8bpp */
	if (pScrn->depth > 8 && pScrn->defaultVisual != TrueColor) {
		xf86DrvMsg(pScrn->scrnIndex, X_ERROR, "requested default visual"
			   " (%s) is not supported at depth %d\n",
			   xf86GetVisualName(pScrn->defaultVisual), pScrn->depth);
		return FALSE;
	}

	{
		Gamma zeros = {0.0, 0.0, 0.0};

		if (!xf86SetGamma(pScrn,zeros)) {
			return FALSE;
		}
	}

	pScrn->progClock = TRUE;
	pScrn->rgbBits   = 8;
	pScrn->chipset   = "fbdev";
	pScrn->videoRam  = fbdevHWGetVidmem(pScrn);

	xf86DrvMsg(pScrn->scrnIndex, X_INFO, "hardware: %s (video memory:"
		   " %dkB)\n", fbdevHWGetName(pScrn), pScrn->videoRam/1024);

	/* handle options */
	xf86CollectOptions(pScrn, NULL);
	if (!(fPtr->Options = xalloc(sizeof(SPU_FBDevOptions))))
		return FALSE;
	memcpy(fPtr->Options, SPU_FBDevOptions, sizeof(SPU_FBDevOptions));
	xf86ProcessOptions(pScrn->scrnIndex, fPtr->pEnt->device->options, fPtr->Options);

	/* use shadow framebuffer by default */
	fPtr->shadowFB = xf86ReturnOptValBool(fPtr->Options, OPTION_SHADOW_FB, TRUE);

	debug = xf86ReturnOptValBool(fPtr->Options, OPTION_DEBUG, FALSE);

	/* rotation */
	fPtr->rotate = SPU_FBDEV_ROTATE_NONE;
	if ((s = xf86GetOptValString(fPtr->Options, OPTION_ROTATE)))
	{
	  if(!xf86NameCmp(s, "CW"))
	  {
	    fPtr->shadowFB = TRUE;
	    fPtr->rotate = SPU_FBDEV_ROTATE_CW;
	    xf86DrvMsg(pScrn->scrnIndex, X_CONFIG,
		       "rotating screen clockwise\n");
	  }
	  else if(!xf86NameCmp(s, "CCW"))
	  {
	    fPtr->shadowFB = TRUE;
	    fPtr->rotate = SPU_FBDEV_ROTATE_CCW;
	    xf86DrvMsg(pScrn->scrnIndex, X_CONFIG,
		       "rotating screen counter-clockwise\n");
	  }
	  else if(!xf86NameCmp(s, "UD"))
	  {
	    fPtr->shadowFB = TRUE;
	    fPtr->rotate = SPU_FBDEV_ROTATE_UD;
	    xf86DrvMsg(pScrn->scrnIndex, X_CONFIG,
		       "rotating screen upside-down\n");
	  }
	  else
	  {
	    xf86DrvMsg(pScrn->scrnIndex, X_CONFIG,
		       "\"%s\" is not a valid value for Option \"Rotate\"\n", s);
	    xf86DrvMsg(pScrn->scrnIndex, X_INFO,
		       "valid options are \"CW\", \"CCW\" and \"UD\"\n");
	  }
	}

	/* select video modes */

	xf86DrvMsg(pScrn->scrnIndex, X_INFO, "checking modes against framebuffer device...\n");
	fbdevHWSetVideoModes(pScrn);

	xf86DrvMsg(pScrn->scrnIndex, X_INFO, "checking modes against monitor...\n");
	{
		DisplayModePtr mode, first = mode = pScrn->modes;
		
		if (mode != NULL) do {
			mode->status = xf86CheckModeForMonitor(mode, pScrn->monitor);
			mode = mode->next;
		} while (mode != NULL && mode != first);

		xf86PruneDriverModes(pScrn);
	}

	if (NULL == pScrn->modes)
		fbdevHWUseBuildinMode(pScrn);
	pScrn->currentMode = pScrn->modes;

	/* First approximation, may be refined in ScreenInit */
	pScrn->displayWidth = pScrn->virtualX;

	xf86PrintModes(pScrn);

	/* Set display resolution */
	xf86SetDpi(pScrn, 0, 0);

	/* Load bpp-specific modules */
	switch ((type = fbdevHWGetType(pScrn)))
	{
	case FBDEVHW_PLANES:
		mod = "afb";
		syms = afbSymbols;
		break;
	case FBDEVHW_PACKED_PIXELS:
		switch (pScrn->bitsPerPixel)
		{
		case 8:
		case 16:
		case 24:
		case 32:
			mod = "fb";
			syms = fbSymbols;
			break;
		default:
			xf86DrvMsg(pScrn->scrnIndex, X_ERROR,
			"unsupported number of bits per pixel: %d",
			pScrn->bitsPerPixel);
			return FALSE;
		}
		break;
	case FBDEVHW_INTERLEAVED_PLANES:
               /* Not supported yet, don't know what to do with this */
               xf86DrvMsg(pScrn->scrnIndex, X_ERROR,
                          "interleaved planes are not yet supported by the "
			  "fbdev driver\n");
		return FALSE;
	case FBDEVHW_TEXT:
               /* This should never happen ...
                * we should check for this much much earlier ... */
               xf86DrvMsg(pScrn->scrnIndex, X_ERROR,
                          "text mode is not supported by the fbdev driver\n");
		return FALSE;
       case FBDEVHW_VGA_PLANES:
               /* Not supported yet */
               xf86DrvMsg(pScrn->scrnIndex, X_ERROR,
                          "EGA/VGA planes are not yet supported by the fbdev "
			  "driver\n");
               return FALSE;
       default:
               xf86DrvMsg(pScrn->scrnIndex, X_ERROR,
                          "unrecognised fbdev hardware type (%d)\n", type);
               return FALSE;
	}
	if (mod && xf86LoadSubModule(pScrn, mod) == NULL) {
		SPU_FBDevFreeRec(pScrn);
		return FALSE;
	}
	if (mod && syms) {
		xf86LoaderReqSymLists(syms, NULL);
	}

	/* Load shadow if needed */
	if (fPtr->shadowFB) {
		xf86DrvMsg(pScrn->scrnIndex, X_CONFIG, "using shadow"
			   " framebuffer\n");
		if (!xf86LoadSubModule(pScrn, "shadow")) {
			SPU_FBDevFreeRec(pScrn);
			return FALSE;
		}
		xf86LoaderReqSymLists(shadowSymbols, NULL);
	}

	TRACE_EXIT("PreInit");
	return TRUE;
}


static Bool
SPU_FBDevCreateScreenResources(ScreenPtr pScreen)
{
    PixmapPtr pPixmap;
    ScrnInfoPtr pScrn = xf86Screens[pScreen->myNum];
    SPU_FBDevPtr fPtr = SPU_FBDEVPTR(pScrn);
    Bool ret;

    pScreen->CreateScreenResources = fPtr->CreateScreenResources;
    ret = pScreen->CreateScreenResources(pScreen);
    pScreen->CreateScreenResources = SPU_FBDevCreateScreenResources;

    if (!ret)
	return FALSE;

    pPixmap = pScreen->GetScreenPixmap(pScreen);

    if (!shadowAdd(pScreen, pPixmap, fPtr->rotate ?
		   shadowUpdateRotatePackedWeak() : shadowUpdatePackedWeak(),
		   SPU_FBDevWindowLinear, fPtr->rotate, NULL)) {
	return FALSE;
    }

    return TRUE;
}

static Bool
SPU_FBDevShadowInit(ScreenPtr pScreen)
{
    ScrnInfoPtr pScrn = xf86Screens[pScreen->myNum];
    SPU_FBDevPtr fPtr = SPU_FBDEVPTR(pScrn);
    
    if (!shadowSetup(pScreen)) {
	return FALSE;
    }

    fPtr->CreateScreenResources = pScreen->CreateScreenResources;
    pScreen->CreateScreenResources = SPU_FBDevCreateScreenResources;

    return TRUE;
}


static Bool
SPU_FBDevScreenInit(int scrnIndex, ScreenPtr pScreen, int argc, char **argv)
{
	ScrnInfoPtr pScrn = xf86Screens[pScreen->myNum];
	SPU_FBDevPtr fPtr = SPU_FBDEVPTR(pScrn);
	VisualPtr visual;
	int init_picture = 0;
	int ret, flags;
	int type;

	TRACE_ENTER("SPU_FBDevScreenInit");

#if DEBUG
	ErrorF("\tbitsPerPixel=%d, depth=%d, defaultVisual=%s\n"
	       "\tmask: %x,%x,%x, offset: %d,%d,%d\n",
	       pScrn->bitsPerPixel,
	       pScrn->depth,
	       xf86GetVisualName(pScrn->defaultVisual),
	       pScrn->mask.red,pScrn->mask.green,pScrn->mask.blue,
	       pScrn->offset.red,pScrn->offset.green,pScrn->offset.blue);
#endif

	if (NULL == (fPtr->fbmem = fbdevHWMapVidmem(pScrn))) {
	        xf86DrvMsg(scrnIndex,X_ERROR,"mapping of video memory"
			   " failed\n");
		return FALSE;
	}
	fPtr->fboff = fbdevHWLinearOffset(pScrn);

	fbdevHWSave(pScrn);

	if (!fbdevHWModeInit(pScrn, pScrn->currentMode)) {
		xf86DrvMsg(scrnIndex,X_ERROR,"mode initialization failed\n");
		return FALSE;
	}
	fbdevHWSaveScreen(pScreen, SCREEN_SAVER_ON);
	fbdevHWAdjustFrame(scrnIndex,0,0,0);

	/* mi layer */
	miClearVisualTypes();
	if (pScrn->bitsPerPixel > 8) {
		if (!miSetVisualTypes(pScrn->depth, TrueColorMask, pScrn->rgbBits, TrueColor)) {
			xf86DrvMsg(scrnIndex,X_ERROR,"visual type setup failed"
				   " for %d bits per pixel [1]\n",
				   pScrn->bitsPerPixel);
			return FALSE;
		}
	} else {
		if (!miSetVisualTypes(pScrn->depth,
				      miGetDefaultVisualMask(pScrn->depth),
				      pScrn->rgbBits, pScrn->defaultVisual)) {
			xf86DrvMsg(scrnIndex,X_ERROR,"visual type setup failed"
				   " for %d bits per pixel [2]\n",
				   pScrn->bitsPerPixel);
			return FALSE;
		}
	}
	if (!miSetPixmapDepths()) {
	  xf86DrvMsg(scrnIndex,X_ERROR,"pixmap depth setup failed\n");
	  return FALSE;
	}

	if(fPtr->rotate==SPU_FBDEV_ROTATE_CW || fPtr->rotate==SPU_FBDEV_ROTATE_CCW)
	{
	  int tmp = pScrn->virtualX;
	  pScrn->virtualX = pScrn->displayWidth = pScrn->virtualY;
	  pScrn->virtualY = tmp;
	} else if (!fPtr->shadowFB) {
		/* FIXME: this doesn't work for all cases, e.g. when each scanline
			has a padding which is independent from the depth (controlfb) */
		pScrn->displayWidth = fbdevHWGetLineLength(pScrn) /
				      (pScrn->bitsPerPixel / 8);

		if (pScrn->displayWidth != pScrn->virtualX) {
			xf86DrvMsg(scrnIndex, X_INFO,
				   "Pitch updated to %d after ModeInit\n",
				   pScrn->displayWidth);
		}
	}

	if(fPtr->rotate && !fPtr->PointerMoved) {
		fPtr->PointerMoved = pScrn->PointerMoved;
		pScrn->PointerMoved = SPU_FBDevPointerMoved;
	}

	fPtr->fbstart = fPtr->fbmem + fPtr->fboff;

	if (fPtr->shadowFB) {
	    fPtr->shadow = xcalloc(1, pScrn->virtualX * pScrn->virtualY *
				   pScrn->bitsPerPixel);

	    if (!fPtr->shadow) {
		xf86DrvMsg(pScrn->scrnIndex, X_ERROR,
			   "Failed to allocate shadow framebuffer\n");
		return FALSE;
	    }
	}

	switch ((type = fbdevHWGetType(pScrn)))
	{
#ifdef USE_AFB
	case SPU_FBDEVHW_PLANES:
		if (fPtr->rotate)
		{
		  xf86DrvMsg(scrnIndex, X_ERROR,
			     "internal error: rotate not supported for afb\n");
		  ret = FALSE;
		  break;
		}
		if (fPtr->shadowFB)
		{
		  xf86DrvMsg(scrnIndex, X_ERROR,
			     "internal error: shadow framebuffer not supported"
			     " for afb\n");
		  ret = FALSE;
		  break;
		}
		ret = afbScreenInit
			(pScreen, fPtr->fbstart, pScrn->virtualX, pScrn->virtualY,
			 pScrn->xDpi, pScrn->yDpi, pScrn->displayWidth);
		break;
#endif
	case FBDEVHW_PACKED_PIXELS:
		switch (pScrn->bitsPerPixel) {
		case 8:
		case 16:
		case 24:
		case 32:
			ret = fbScreenInit(pScreen, fPtr->shadowFB ? fPtr->shadow
					   : fPtr->fbstart, pScrn->virtualX,
					   pScrn->virtualY, pScrn->xDpi,
					   pScrn->yDpi, pScrn->displayWidth,
					   pScrn->bitsPerPixel);
			init_picture = 1;
			break;
	 	default:
			xf86DrvMsg(scrnIndex, X_ERROR,
				   "internal error: invalid number of bits per"
				   " pixel (%d) encountered in"
				   " SPU_FBDevScreenInit()\n", pScrn->bitsPerPixel);
			ret = FALSE;
			break;
		}
		break;
	case FBDEVHW_INTERLEAVED_PLANES:
		/* This should never happen ...
		* we should check for this much much earlier ... */
		xf86DrvMsg(scrnIndex, X_ERROR,
		           "internal error: interleaved planes are not yet "
			   "supported by the fbdev driver\n");
		ret = FALSE;
		break;
	case FBDEVHW_TEXT:
		/* This should never happen ...
		* we should check for this much much earlier ... */
		xf86DrvMsg(scrnIndex, X_ERROR,
		           "internal error: text mode is not supported by the "
			   "fbdev driver\n");
		ret = FALSE;
		break;
	case FBDEVHW_VGA_PLANES:
		/* Not supported yet */
		xf86DrvMsg(scrnIndex, X_ERROR,
		           "internal error: EGA/VGA Planes are not yet "
			   "supported by the fbdev driver\n");
		ret = FALSE;
		break;
	default:
		xf86DrvMsg(scrnIndex, X_ERROR,
		           "internal error: unrecognised hardware type (%d) "
			   "encountered in SPU_FBDevScreenInit()\n", type);
		ret = FALSE;
		break;
	}
	if (!ret)
		return FALSE;

	if (pScrn->bitsPerPixel > 8) {
		/* Fixup RGB ordering */
		visual = pScreen->visuals + pScreen->numVisuals;
		while (--visual >= pScreen->visuals) {
			if ((visual->class | DynamicClass) == DirectColor) {
				visual->offsetRed   = pScrn->offset.red;
				visual->offsetGreen = pScrn->offset.green;
				visual->offsetBlue  = pScrn->offset.blue;
				visual->redMask     = pScrn->mask.red;
				visual->greenMask   = pScrn->mask.green;
				visual->blueMask    = pScrn->mask.blue;
			}
		}
	}

	/* must be after RGB ordering fixed */
	if (init_picture && !fbPictureInit(pScreen, NULL, 0))
		xf86DrvMsg(pScrn->scrnIndex, X_WARNING,
			   "Render extension initialisation failed\n");

	if (fPtr->shadowFB && !SPU_FBDevShadowInit(pScreen)) {
	    xf86DrvMsg(scrnIndex, X_ERROR,
		       "shadow framebuffer initialization failed\n");
	    return FALSE;
	}

	if (!fPtr->rotate)
	  SPU_FBDevDGAInit(pScrn, pScreen);
	else {
	  xf86DrvMsg(scrnIndex, X_INFO, "display rotated; disabling DGA\n");
	  xf86DrvMsg(scrnIndex, X_INFO, "using driver rotation; disabling "
			                "XRandR\n");
	  xf86DisableRandR();
	  if (pScrn->bitsPerPixel == 24)
	    xf86DrvMsg(scrnIndex, X_WARNING, "rotation might be broken at 24 "
                                             "bits per pixel\n");
	}

	xf86SetBlackWhitePixels(pScreen);
	miInitializeBackingStore(pScreen);
	xf86SetBackingStore(pScreen);

	/* software cursor */
	miDCInitialize(pScreen, xf86GetPointerScreenFuncs());

	/* colormap */
	switch ((type = fbdevHWGetType(pScrn)))
	{
	/* XXX It would be simpler to use miCreateDefColormap() in all cases. */
#ifdef USE_AFB
	case FBDEVHW_PLANES:
		if (!afbCreateDefColormap(pScreen)) {
			xf86DrvMsg(scrnIndex, X_ERROR,
                                   "internal error: afbCreateDefColormap "
				   "failed in SPU_FBDevScreenInit()\n");
			return FALSE;
		}
		break;
#endif
	case FBDEVHW_PACKED_PIXELS:
		if (!miCreateDefColormap(pScreen)) {
			xf86DrvMsg(scrnIndex, X_ERROR,
                                   "internal error: miCreateDefColormap failed "
				   "in SPU_FBDevScreenInit()\n");
			return FALSE;
		}
		break;
	case FBDEVHW_INTERLEAVED_PLANES:
		xf86DrvMsg(scrnIndex, X_ERROR,
		           "internal error: interleaved planes are not yet "
			   "supported by the fbdev driver\n");
		return FALSE;
	case FBDEVHW_TEXT:
		xf86DrvMsg(scrnIndex, X_ERROR,
		           "internal error: text mode is not supported by "
			   "the fbdev driver\n");
		return FALSE;
	case FBDEVHW_VGA_PLANES:
		xf86DrvMsg(scrnIndex, X_ERROR,
		           "internal error: EGA/VGA planes are not yet "
			   "supported by the fbdev driver\n");
		return FALSE;
	default:
		xf86DrvMsg(scrnIndex, X_ERROR,
		           "internal error: unrecognised fbdev hardware type "
			   "(%d) encountered in SPU_FBDevScreenInit()\n", type);
		return FALSE;
	}
	flags = CMAP_PALETTED_TRUECOLOR;
	if(!xf86HandleColormaps(pScreen, 256, 8, fbdevHWLoadPaletteWeak(), 
				NULL, flags))
		return FALSE;

	xf86DPMSInit(pScreen, fbdevHWDPMSSetWeak(), 0);

	pScreen->SaveScreen = fbdevHWSaveScreenWeak();
	
	SPUInitVideo(pScreen);

	/* Wrap the current CloseScreen function */
	fPtr->CloseScreen = pScreen->CloseScreen;
	pScreen->CloseScreen = SPU_FBDevCloseScreen;

// 	{
// 	    XF86VideoAdaptorPtr *ptr;
// 
// 	    int n = xf86XVListGenericAdaptors(pScrn,&ptr);
// 	    if (n) {
// 		xf86XVScreenInit(pScreen,ptr,n);
// 	    }
// 	}

	

	TRACE_EXIT("SPU_FBDevScreenInit");

	return TRUE;
}

static Bool
SPU_FBDevCloseScreen(int scrnIndex, ScreenPtr pScreen)
{
	ScrnInfoPtr pScrn = xf86Screens[scrnIndex];
	SPU_FBDevPtr fPtr = SPU_FBDEVPTR(pScrn);
	
	fbdevHWRestore(pScrn);
	fbdevHWUnmapVidmem(pScrn);
	if (fPtr->shadow) {
	    xfree(fPtr->shadow);
	    fPtr->shadow = NULL;
	}
	if (fPtr->pDGAMode) {
	  xfree(fPtr->pDGAMode);
	  fPtr->pDGAMode = NULL;
	  fPtr->nDGAMode = 0;
	}
	pScrn->vtSema = FALSE;

	pScreen->CreateScreenResources = fPtr->CreateScreenResources;
	pScreen->CloseScreen = fPtr->CloseScreen;
	return (*pScreen->CloseScreen)(scrnIndex, pScreen);
}



/***********************************************************************
 * Shadow stuff
 ***********************************************************************/

static void *
SPU_FBDevWindowLinear(ScreenPtr pScreen, CARD32 row, CARD32 offset, int mode,
		 CARD32 *size, void *closure)
{
    ScrnInfoPtr pScrn = xf86Screens[pScreen->myNum];
    SPU_FBDevPtr fPtr = SPU_FBDEVPTR(pScrn);

    if (!pScrn->vtSema)
      return NULL;

    if (fPtr->lineLength)
      *size = fPtr->lineLength;
    else
      *size = fPtr->lineLength = fbdevHWGetLineLength(pScrn);

    return ((CARD8 *)fPtr->fbstart + row * fPtr->lineLength + offset);
}

static void
SPU_FBDevPointerMoved(int index, int x, int y)
{
    ScrnInfoPtr pScrn = xf86Screens[index];
    SPU_FBDevPtr fPtr = SPU_FBDEVPTR(pScrn);
    int newX, newY;

    switch (fPtr->rotate)
    {
    case SPU_FBDEV_ROTATE_CW:
	/* 90 degrees CW rotation. */
	newX = pScrn->pScreen->height - y - 1;
	newY = x;
	break;

    case SPU_FBDEV_ROTATE_CCW:
	/* 90 degrees CCW rotation. */
	newX = y;
	newY = pScrn->pScreen->width - x - 1;
	break;

    case SPU_FBDEV_ROTATE_UD:
	/* 180 degrees UD rotation. */
	newX = pScrn->pScreen->width - x - 1;
	newY = pScrn->pScreen->height - y - 1;
	break;

    default:
	/* No rotation. */
	newX = x;
	newY = y;
	break;
    }

    /* Pass adjusted pointer coordinates to wrapped PointerMoved function. */
    (*fPtr->PointerMoved)(index, newX, newY);
}


/***********************************************************************
 * DGA stuff
 ***********************************************************************/
static Bool SPU_FBDevDGAOpenFramebuffer(ScrnInfoPtr pScrn, char **DeviceName,
				   unsigned char **ApertureBase,
				   int *ApertureSize, int *ApertureOffset,
				   int *flags);
static Bool SPU_FBDevDGASetMode(ScrnInfoPtr pScrn, DGAModePtr pDGAMode);
static void SPU_FBDevDGASetViewport(ScrnInfoPtr pScrn, int x, int y, int flags);

static Bool
SPU_FBDevDGAOpenFramebuffer(ScrnInfoPtr pScrn, char **DeviceName,
		       unsigned char **ApertureBase, int *ApertureSize,
		       int *ApertureOffset, int *flags)
{
    *DeviceName = NULL;		/* No special device */
    *ApertureBase = (unsigned char *)(pScrn->memPhysBase);
    *ApertureSize = pScrn->videoRam;
    *ApertureOffset = pScrn->fbOffset;
    *flags = 0;

    return TRUE;
}

static Bool
SPU_FBDevDGASetMode(ScrnInfoPtr pScrn, DGAModePtr pDGAMode)
{
    DisplayModePtr pMode;
    int scrnIdx = pScrn->pScreen->myNum;
    int frameX0, frameY0;

    if (pDGAMode) {
	pMode = pDGAMode->mode;
	frameX0 = frameY0 = 0;
    }
    else {
	if (!(pMode = pScrn->currentMode))
	    return TRUE;

	frameX0 = pScrn->frameX0;
	frameY0 = pScrn->frameY0;
    }

    if (!(*pScrn->SwitchMode)(scrnIdx, pMode, 0))
	return FALSE;
    (*pScrn->AdjustFrame)(scrnIdx, frameX0, frameY0, 0);

    return TRUE;
}

static void
SPU_FBDevDGASetViewport(ScrnInfoPtr pScrn, int x, int y, int flags)
{
    (*pScrn->AdjustFrame)(pScrn->pScreen->myNum, x, y, flags);
}

static int
SPU_FBDevDGAGetViewport(ScrnInfoPtr pScrn)
{
    return (0);
}

static DGAFunctionRec SPU_FBDevDGAFunctions =
{
    SPU_FBDevDGAOpenFramebuffer,
    NULL,       /* CloseFramebuffer */
    SPU_FBDevDGASetMode,
    SPU_FBDevDGASetViewport,
    SPU_FBDevDGAGetViewport,
    NULL,       /* Sync */
    NULL,       /* FillRect */
    NULL,       /* BlitRect */
    NULL,       /* BlitTransRect */
};

static void
SPU_FBDevDGAAddModes(ScrnInfoPtr pScrn)
{
    SPU_FBDevPtr fPtr = SPU_FBDEVPTR(pScrn);
    DisplayModePtr pMode = pScrn->modes;
    DGAModePtr pDGAMode;

    do {
	pDGAMode = xrealloc(fPtr->pDGAMode,
			    (fPtr->nDGAMode + 1) * sizeof(DGAModeRec));
	if (!pDGAMode)
	    break;

	fPtr->pDGAMode = pDGAMode;
	pDGAMode += fPtr->nDGAMode;
	(void)memset(pDGAMode, 0, sizeof(DGAModeRec));

	++fPtr->nDGAMode;
	pDGAMode->mode = pMode;
	pDGAMode->flags = DGA_CONCURRENT_ACCESS | DGA_PIXMAP_AVAILABLE;
	pDGAMode->byteOrder = pScrn->imageByteOrder;
	pDGAMode->depth = pScrn->depth;
	pDGAMode->bitsPerPixel = pScrn->bitsPerPixel;
	pDGAMode->red_mask = pScrn->mask.red;
	pDGAMode->green_mask = pScrn->mask.green;
	pDGAMode->blue_mask = pScrn->mask.blue;
	pDGAMode->visualClass = pScrn->bitsPerPixel > 8 ?
	    TrueColor : PseudoColor;
	pDGAMode->xViewportStep = 1;
	pDGAMode->yViewportStep = 1;
	pDGAMode->viewportWidth = pMode->HDisplay;
	pDGAMode->viewportHeight = pMode->VDisplay;

	if (fPtr->lineLength)
	  pDGAMode->bytesPerScanline = fPtr->lineLength;
	else
	  pDGAMode->bytesPerScanline = fPtr->lineLength = fbdevHWGetLineLength(pScrn);

	pDGAMode->imageWidth = pMode->HDisplay;
	pDGAMode->imageHeight =  pMode->VDisplay;
	pDGAMode->pixmapWidth = pDGAMode->imageWidth;
	pDGAMode->pixmapHeight = pDGAMode->imageHeight;
	pDGAMode->maxViewportX = pScrn->virtualX -
				    pDGAMode->viewportWidth;
	pDGAMode->maxViewportY = pScrn->virtualY -
				    pDGAMode->viewportHeight;

	pDGAMode->address = fPtr->fbstart;

	pMode = pMode->next;
    } while (pMode != pScrn->modes);
}

static Bool
SPU_FBDevDGAInit(ScrnInfoPtr pScrn, ScreenPtr pScreen)
{
    SPU_FBDevPtr fPtr = SPU_FBDEVPTR(pScrn);

    if (pScrn->depth < 8)
	return FALSE;

    if (!fPtr->nDGAMode)
	SPU_FBDevDGAAddModes(pScrn);

    return (DGAInit(pScreen, &SPU_FBDevDGAFunctions,
	    fPtr->pDGAMode, fPtr->nDGAMode));
}

static Bool
SPU_FBDevDriverFunc(ScrnInfoPtr pScrn, xorgDriverFuncOp op, pointer ptr)
{
    xorgHWFlags *flag;
    
    switch (op) {
	case GET_REQUIRED_HW_INTERFACES:
	    flag = (CARD32*)ptr;
	    (*flag) = 0;
	    return TRUE;
	default:
	    return FALSE;
    }
}
