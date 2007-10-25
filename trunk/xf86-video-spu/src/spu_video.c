
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <string.h>

#include "xf86.h"
#include "dixstruct.h"

#include <xf86fbman.h>

#include "xf86xv.h"
#include <X11/extensions/Xv.h>
#include <spu-medialib/yuv2argb_scaler.h>
#include <spu-medialib/spu_control.h>

#include "fourcc.h"
//FIXME USE THE TIMER LUKE..
#define OFF_DELAY       250  /* milliseconds */
#define FREE_DELAY      15000

#define OFF_TIMER       0x01
#define FREE_TIMER      0x02
#define CLIENT_VIDEO_ON 0x04

#define TIMER_MASK      (OFF_TIMER | FREE_TIMER) 
//FIXME USE THE TIMER LUKE.. 

static XF86VideoAdaptorPtr SPUSetupImageVideo(ScreenPtr);
static int  SPUSetPortAttribute(ScrnInfoPtr, Atom, INT32, pointer);
static int  SPUGetPortAttribute(ScrnInfoPtr, Atom ,INT32 *, pointer);
static void SPUStopVideo(ScrnInfoPtr, pointer, Bool);
static void SPUQueryBestSize(ScrnInfoPtr, Bool, short, short, short, short,
			unsigned int *, unsigned int *, pointer);
static int  SPUPutImage(ScrnInfoPtr, short, short, short, short, short,
			short, short, short, int, unsigned char*, short,
			short, Bool, RegionPtr, pointer, DrawablePtr);
static int  SPUQueryImageAttributes(ScrnInfoPtr, int, unsigned short *,
			unsigned short *,  int *, int *);


static void SPUVideoTimerCallback(ScrnInfoPtr pScrn, Time now);


#define MAKE_ATOM(a) MakeAtom(a, sizeof(a) - 1, TRUE)

static Atom xvBrightness, xvColorKey, xvSaturation, xvDoubleBuffer;

typedef struct {
	XF86VideoAdaptorPtr adaptor;
	void              (*VideoTimerCallback)(ScrnInfoPtr, Time);
	int               videoKey;

} SPUInfoRec, *SPUInfoPtr; //TODO merge with the port priv ..

#define SPUPTR(pScrn) ((SPUInfoPtr)(pScrn)->driverPrivate)

typedef struct {
   int           brightness;
   int           saturation;
   Bool          doubleBuffer;
   unsigned char currentBuffer;
   FBLinearPtr  linear; 
   char 	*BufferPointer;
   RegionRec     clip;
   CARD32        colorKey;
   CARD32        videoStatus;
   Time          offTime;
   Time          freeTime;
   int           ecp_div;
   int		old_w;
   int		old_h;
   int		old_x;
   int 		old_y;
   int 		running;
   char *Ypointer[2];
   char *Upointer[2]; 
   char *Vpointer[2];
   void *Opointer[2];
   yuvscaler2argb_t *yuv4202argb;
   
} SPUPortPrivRec, *SPUPortPrivPtr;


void SPUInitVideo(ScreenPtr pScreen)
{
    ScrnInfoPtr pScrn = xf86Screens[pScreen->myNum];
    XF86VideoAdaptorPtr *adaptors, *newAdaptors = NULL;
    XF86VideoAdaptorPtr newAdaptor = NULL;
    int num_adaptors;

    newAdaptor = SPUSetupImageVideo(pScreen);

    num_adaptors = xf86XVListGenericAdaptors(pScrn, &adaptors);

    if(newAdaptor) {
	if(!num_adaptors) {
	    num_adaptors = 1;
	    adaptors = &newAdaptor;
	} else {
	    newAdaptors =  /* need to free this someplace */
		xalloc((num_adaptors + 1) * sizeof(XF86VideoAdaptorPtr*));
	    if(newAdaptors) {
		memcpy(newAdaptors, adaptors, num_adaptors *
					sizeof(XF86VideoAdaptorPtr));
		newAdaptors[num_adaptors] = newAdaptor;
		adaptors = newAdaptors;
		num_adaptors++;
	    }
	}
    }

    if(num_adaptors)
	xf86XVScreenInit(pScreen, adaptors, num_adaptors);

    if(newAdaptors)
	xfree(newAdaptors);
}

#define MAXWIDTH 1920
#define MAXHEIGHT 1080

/* client libraries expect an encoding */
static XF86VideoEncodingRec DummyEncoding =
{
   0,
   "XV_IMAGE",
   MAXWIDTH, MAXHEIGHT,
   {1, 1}
};

#define NUM_FORMATS 12

static XF86VideoFormatRec Formats[NUM_FORMATS] =
{
   {8, TrueColor}, {8, DirectColor}, {8, PseudoColor},
   {8, GrayScale}, {8, StaticGray}, {8, StaticColor},
   {15, TrueColor}, {16, TrueColor}, {24, TrueColor},
   {15, DirectColor}, {16, DirectColor}, {24, DirectColor}
};


#define NUM_ATTRIBUTES 4

static XF86AttributeRec Attributes[NUM_ATTRIBUTES] =
{
   {XvSettable | XvGettable, 0, (1 << 24) - 1, "XV_COLORKEY"},
   {XvSettable | XvGettable, -64, 63, "XV_BRIGHTNESS"},
   {XvSettable | XvGettable, 0, 31, "XV_SATURATION"},
   {XvGettable,0, 1, "XV_DOUBLE_BUFFER"}
};

#define NUM_IMAGES 4

static XF86ImageRec Images[NUM_IMAGES] =
{
	XVIMAGE_YUY2, //FIXME not yet implemented
	XVIMAGE_UYVY, // FIXME not yet implemented
	XVIMAGE_YV12,
	XVIMAGE_I420
};

static XF86VideoAdaptorPtr
SPUAllocAdaptor(ScrnInfoPtr pScrn)
{
    XF86VideoAdaptorPtr adapt;
    SPUInfoPtr info = SPUPTR(pScrn);
    SPUPortPrivPtr pPriv;

    if(!(adapt = xf86XVAllocateVideoAdaptorRec(pScrn)))
	return NULL;

    if(!(pPriv = xcalloc(1, sizeof(SPUPortPrivRec) + sizeof(DevUnion))))
    {
	xfree(adapt);
	return NULL;
    }

    adapt->pPortPrivates = (DevUnion*)(&pPriv[1]);
    adapt->pPortPrivates[0].ptr = (pointer)pPriv;

    xvBrightness   = MAKE_ATOM("XV_BRIGHTNESS");
    xvSaturation   = MAKE_ATOM("XV_SATURATION");
    xvColorKey     = MAKE_ATOM("XV_COLORKEY");
    xvDoubleBuffer = MAKE_ATOM("XV_DOUBLE_BUFFER");

    pPriv->colorKey =info->videoKey;
    pPriv->doubleBuffer = TRUE;
    pPriv->videoStatus = 0;
    pPriv->brightness = 0;
    pPriv->saturation = 16;
    pPriv->currentBuffer = 0;

    pPriv->running=0;


    return adapt;
}

static XF86VideoAdaptorPtr
SPUSetupImageVideo(ScreenPtr pScreen)
{
    ScrnInfoPtr pScrn = xf86Screens[pScreen->myNum];
    SPUInfoPtr info = SPUPTR(pScrn);
    SPUPortPrivPtr pPriv;
    XF86VideoAdaptorPtr adapt;

    if(!(adapt = SPUAllocAdaptor(pScrn)))
	return NULL;

    adapt->type = XvWindowMask | XvInputMask | XvImageMask;
    adapt->flags =0;// VIDEO_OVERLAID_IMAGES | VIDEO_CLIP_TO_VIEWPORT;
    adapt->name = "SPU Video Acelleration Layer";
    adapt->nEncodings = 1;
    adapt->pEncodings = &DummyEncoding;
    adapt->nFormats = NUM_FORMATS;
    adapt->pFormats = Formats;
    adapt->nPorts = 1;
    adapt->nAttributes = NUM_ATTRIBUTES;
    adapt->pAttributes = Attributes;
    adapt->nImages = NUM_IMAGES;
    adapt->pImages = Images;
    adapt->PutVideo = NULL;
    adapt->PutStill = NULL;
    adapt->GetVideo = NULL;
    adapt->GetStill = NULL;
    adapt->StopVideo = SPUStopVideo;
    adapt->SetPortAttribute = SPUSetPortAttribute;
    adapt->GetPortAttribute = SPUGetPortAttribute;
    adapt->QueryBestSize = SPUQueryBestSize;
    adapt->PutImage = SPUPutImage;
    adapt->QueryImageAttributes = SPUQueryImageAttributes;

    info->adaptor = adapt;
    pPriv = (SPUPortPrivPtr)(adapt->pPortPrivates[0].ptr);
   
    REGION_NULL(pScreen, &(pPriv->clip));

    return adapt;
}

static void
SPUStopVideo(ScrnInfoPtr pScrn, pointer data, Bool cleanup)
{
  SPUInfoPtr info = SPUPTR(pScrn);
  SPUPortPrivPtr pPriv = (SPUPortPrivPtr)data;

  REGION_EMPTY(pScrn->pScreen, &pPriv->clip);
	
   if(cleanup) {
    //  if(pPriv->videoStatus & CLIENT_VIDEO_ON) {

      //}
      if(pPriv->linear) {
 	free(pPriv->linear);
 	pPriv->linear = NULL;
      }
     	 pPriv->videoStatus = 0;
	 pPriv->running=0;
     	 yuvscsc_send_message(pPriv->yuv4202argb,STOP);
      
   } else {
      if(pPriv->videoStatus & CLIENT_VIDEO_ON) {
 	pPriv->videoStatus |= OFF_TIMER;
	pPriv->videoStatus = 0;
// 	pPriv->offTime = currentTime.milliseconds + OFF_DELAY; //FIXME fix timing issues.. 
      }
   }
}

static int //FIXME this can probably be reduced to next to nothing
SPUSetPortAttribute(
  ScrnInfoPtr pScrn,
  Atom attribute,
  INT32 value,
  pointer data
){
  SPUInfoPtr info = SPUPTR(pScrn);
 // unsigned char *SPUMMIO = info->MMIO;
  SPUPortPrivPtr pPriv = (SPUPortPrivPtr)data;

  if(attribute == xvBrightness) {
	if((value < -64) || (value > 63))
	   return BadValue;
	pPriv->brightness = value;

  } else
  if(attribute == xvSaturation) {
	if((value < 0) || (value > 31))
	   return BadValue;
	pPriv->saturation = value;

  } else
  if(attribute == xvDoubleBuffer) {
	if((value < 0) || (value > 1))
	   return BadValue;
	pPriv->doubleBuffer = value;
  } else
  if(attribute == xvColorKey) {
	pPriv->colorKey = value;

	REGION_EMPTY(pScrn->pScreen, &pPriv->clip);
  } else return BadMatch;

  return Success;
}

static int
SPUGetPortAttribute(
  ScrnInfoPtr pScrn,
  Atom attribute,
  INT32 *value,
  pointer data
){
  SPUPortPrivPtr pPriv = (SPUPortPrivPtr)data;

  if(attribute == xvBrightness) {
	*value = pPriv->brightness;
  } else
  if(attribute == xvSaturation) {
	*value = pPriv->saturation;
  } else
  if(attribute == xvDoubleBuffer) {
	*value = pPriv->doubleBuffer ? 1 : 0;
  } else
  if(attribute == xvColorKey) {
	*value = pPriv->colorKey;
  } else return BadMatch;

  return Success;
}


static void
SPUQueryBestSize(
  ScrnInfoPtr pScrn,
  Bool motion,
  short vid_w, short vid_h,
  short drw_w, short drw_h,
  unsigned int *p_w, unsigned int *p_h,
  pointer data
){
   if(vid_w > (drw_w << 4))
	drw_w = vid_w >> 4;
   if(vid_h > (drw_h << 4))
	drw_h = vid_h >> 4;

  *p_w = drw_w;
  *p_h = drw_h;
}


static int
SPUPutImage(
  ScrnInfoPtr pScrn,
  short src_x, short src_y,
  short drw_x, short drw_y,
  short src_w, short src_h,
  short drw_w, short drw_h,
  int id, unsigned char* buf,
  short width, short height,
  Bool Sync,
  RegionPtr clipBoxes, pointer data,
  DrawablePtr pDraw
){
   SPUInfoPtr info = SPUPTR(pScrn);
   SPUPortPrivPtr pPriv = (SPUPortPrivPtr)data;
    INT32 xa, xb, ya, yb;
	int offset;
	char* tmp;

   BoxRec dstBox;




   if(src_w > (drw_w << 4))
	drw_w = src_w >> 4;
   if(src_h > (drw_h << 4))
	drw_h = src_h >> 4;


   /* Clip */
   xa = src_x;
   xb = src_x + src_w;
   ya = src_y;
   yb = src_y + src_h;

   dstBox.x1 = drw_x;
   dstBox.x2 = drw_x + drw_w;
   dstBox.y1 = drw_y;
   dstBox.y2 = drw_y + drw_h;

   if(!xf86XVClipVideoHelper(&dstBox, &xa, &xb, &ya, &yb,
			     clipBoxes, width, height))
	return Success;

   dstBox.x1 -= pScrn->frameX0;
   dstBox.x2 -= pScrn->frameX0;
   dstBox.y1 -= pScrn->frameY0;
   dstBox.y2 -= pScrn->frameY0;
   

    if(!REGION_EQUAL(pScrn->pScreen, &pPriv->clip, clipBoxes)) {
	REGION_COPY(pScrn->pScreen, &pPriv->clip, clipBoxes);
	/* draw these */
	xf86XVFillKeyHelper(pScrn->pScreen, pPriv->colorKey, clipBoxes);
    }
   switch(id) {
    case FOURCC_YV12:
    case FOURCC_I420:

	pPriv->Ypointer[pPriv->currentBuffer]=buf;
	pPriv->Upointer[pPriv->currentBuffer]=buf+src_w*src_h;
	pPriv->Vpointer[pPriv->currentBuffer]=buf+src_w*src_h+(src_w*src_h)/4;
	
	if (id == FOURCC_YV12)
	{
		tmp=pPriv->Upointer[pPriv->currentBuffer];
		pPriv->Upointer[pPriv->currentBuffer]=pPriv->Vpointer[pPriv->currentBuffer];
		pPriv->Vpointer[pPriv->currentBuffer]=tmp;
	}

	PixmapPtr mypixmap;
	mypixmap = pDraw->pScreen->GetWindowPixmap((WindowPtr)pDraw);
	pPriv->Opointer[pPriv->currentBuffer]=mypixmap->devPrivate.ptr;

	int maxwidth=mypixmap->devKind/4;

	if (((pPriv->old_w!=drw_w)||(pPriv->old_h!=drw_h)||(pPriv->old_x!=drw_x)||(pPriv->old_y!=drw_y))&& (pPriv->running==1))
	{
		// the image has moved or changed size..
		yuvscsc_set_srcW(pPriv->yuv4202argb,16*(src_w>>4)); //source needs to be 16 byte 16 pixels width aligned 
		yuvscsc_set_srcH(pPriv->yuv4202argb,16*(src_h>>4));
		yuvscsc_set_dstW(pPriv->yuv4202argb,4*(drw_w>>2)); //destination needs to be 16 byte 4 pixels width aligned
		yuvscsc_set_dstH(pPriv->yuv4202argb,4*(drw_h>>2)); 
		yuvscsc_set_maxwidth(pPriv->yuv4202argb,maxwidth);
		yuvscsc_set_offset(pPriv->yuv4202argb,maxwidth*drw_y+4*(drw_x>>2));
		pPriv->old_w=drw_w;
		pPriv->old_h=drw_h;
		pPriv->old_x=drw_x;
		pPriv->old_y=drw_y;
		
	}

	

	if (!pPriv->running)	{
		pPriv->running=1;
		pPriv->old_w=drw_w;
		pPriv->old_h=drw_h;
		pPriv->old_x=drw_x;
		pPriv->old_y=drw_y;
		
		pPriv->yuv4202argb = yuvscsc_init_yuv2argb_scaler(src_w, src_h, 4*(drw_w>>2), 4*(drw_h>>2), maxwidth*drw_y+4*(drw_x>>2),maxwidth , 
								(ea_t)pPriv->Ypointer[pPriv->currentBuffer], (ea_t)pPriv->Ypointer[pPriv->currentBuffer], 
								(ea_t)pPriv->Upointer[pPriv->currentBuffer], (ea_t)pPriv->Upointer[pPriv->currentBuffer], 
								(ea_t)pPriv->Vpointer[pPriv->currentBuffer], (ea_t)pPriv->Vpointer[pPriv->currentBuffer], 
								pPriv->Opointer[pPriv->currentBuffer],pPriv->Opointer[pPriv->currentBuffer]);

		
		yuvscsc_send_message(pPriv->yuv4202argb,RUN);


	} else {
		//spu is running so lets have it do another frame
		// FIXME ALLWAYS SEND UPDATE ??

		yuvscsc_set_Buffers(pPriv->yuv4202argb, pPriv->Ypointer[pPriv->currentBuffer], pPriv->Upointer[pPriv->currentBuffer], pPriv->Vpointer[pPriv->currentBuffer], pPriv->Opointer[pPriv->currentBuffer],pPriv->currentBuffer);
		yuvscsc_send_message(pPriv->yuv4202argb,UPDATE);
	
		yuvscsc_send_message(pPriv->yuv4202argb,RUN);
	}

	break;
    case FOURCC_UYVY:
    case FOURCC_YUY2:
    default:
//FIXME INSERT SPU FUNCTION FOR YUYV / YUV2 here..

	break;
    }



	pPriv->currentBuffer ^= 1; //may become  useful when spu'ing  flip the buffer please
	pPriv->videoStatus = CLIENT_VIDEO_ON;
	
	if (pPriv->running)
	{
		int spu_msg=0;
		spu_msg=yuvscsc_receive_message(pPriv->yuv4202argb);	 //cmon man this will run forever..FIX INTERRUPT STUFF	
		if (spu_msg == RDY)
		{
			xf86DrvMsg(pScrn->scrnIndex, X_INFO, "Message Recieved\n");
			pPriv->videoStatus = 0;	
		} else {
			return -1;
		}
	}
	info->VideoTimerCallback = SPUVideoTimerCallback;

    return Success;
}


static int
SPUQueryImageAttributes(
    ScrnInfoPtr pScrn,
    int id,
    unsigned short *w, unsigned short *h,
    int *pitches, int *offsets
){
    int size, tmp;

    if(*w > MAXWIDTH) *w = MAXWIDTH;
    if(*h > MAXHEIGHT) *h = MAXHEIGHT;

    *w = (*w + 1) & ~1;
    if(offsets) offsets[0] = 0;

    switch(id) {
    case FOURCC_YV12:
    case FOURCC_I420:
	*h = (*h + 1) & ~1;
	size = (*w + 3) & ~3;
	if(pitches) pitches[0] = size;
	size *= *h;
	if(offsets) offsets[1] = size;
	tmp = ((*w >> 1) + 3) & ~3;
	if(pitches) pitches[1] = pitches[2] = tmp;
	tmp *= (*h >> 1);
	size += tmp;
	if(offsets) offsets[2] = size;
	size += tmp;
	break;
    case FOURCC_UYVY:
    case FOURCC_YUY2:
    default:
	size = *w << 1;
	if(pitches) pitches[0] = size;
	size *= *h;
	break;
    }

    return size;
}

static void
SPUVideoTimerCallback(ScrnInfoPtr pScrn, Time now)
{
    SPUInfoPtr info = SPUPTR(pScrn);
    SPUPortPrivPtr pPriv = info->adaptor->pPortPrivates[0].ptr;

    if(pPriv->videoStatus & TIMER_MASK) {
	if(pPriv->videoStatus & OFF_TIMER) {
	    if(pPriv->offTime < now) {

		pPriv->videoStatus = FREE_TIMER;
		pPriv->freeTime = now + FREE_DELAY;
	    }
	} else {  /* FREE_TIMER */
	    if(pPriv->freeTime < now) {
		if(pPriv->linear) {
		   xf86FreeOffscreenLinear(pPriv->linear);
		   pPriv->linear = NULL;
		}
		pPriv->videoStatus = 0;
		info->VideoTimerCallback = NULL;
	    }
	}
    } else  /* shouldn't get here */
	info->VideoTimerCallback = NULL;
}