#pragma once

struct SStrng
{
	const xmlChar *ptr;
	SStrng( xmlChar const *_ptr = 0 ) : ptr( _ptr )
	{
		if( ptr && *ptr == '#' )++ptr;
	};
	bool operator < (const  SStrng & str ) const
	{
		return xmlStrcmp ( ptr, str.ptr ) < 0;
	};
};


template<typename T> class CAutoContainer
{
		static std::map< SStrng, T*> container;
		static std::map< SStrng, T*> containersid;
		static std::vector<T*>       arrayContainer;


		const xmlChar *myId;
	public:
		void SetId( const xmlChar *id )
		{
			myId = id;
		}

		static void Addsid( const xmlChar * id, T *obj )
		{
			if( obj == 0 || id == 0 )
			{
				return;
			}
			containersid[SStrng(id)] = obj;
		};
		static void Add( const xmlChar * id, T *obj )
		{
			if( obj == 0 )
			{
				return;
			};
			arrayContainer.push_back( obj );
			if( id == 0 )
			{
				return;
			}
			container[SStrng(id)] = obj;
		};

		static const std::vector<T*> &GetArrayContainer()
		{
			return arrayContainer;
		}

		static T *Get(  const xmlChar * id )
		{
			class std::map<SStrng, T*>::iterator it = container.find( id );
			if( it != container.end() )
				return it->second;
			return 0;

		};

		static T *Getsid(  const xmlChar * id )
		{
			class std::map<SStrng, T*>::iterator it = containersid.find( id );
			if( it != containersid.end() )
				return it->second;
			return 0;

		};

		static void Clear()
		{
			for( class std::map< SStrng, T*>::iterator it = container.begin();
			        it != container.end();
			        ++it)
			{
				delete it->second;
			}

			container.clear();
			arrayContainer.clear();
		}
};

#define _X (const xmlChar *)

inline const xmlAttr *Find( const xmlAttr *root, const xmlChar *name )
{
	for( const xmlAttr *node = root; node; node = node->next )
	{
		if( !xmlStrcmp( name, node->name ) )
		{
			return node;
		}
	}
	return 0;
};

inline const xmlNode *Find( const  xmlNode * root, const xmlChar *name )
{
	for ( ;root ;root = root->next )
	{
		if( !xmlStrcmp( name, root->name ) )
		{
			return root;
		}
	}
	return 0;
};

inline const xmlChar *LoadStrAttr(  const xmlNode * root, const xmlChar *name )
{
	const xmlAttr *node = Find( root->properties, name );
	if( !node )
	{
		return 0;
	}
	return node->children->content;
};


bool inline IsSpace( xmlChar v )
{
	return ( v == ' ' ) || ( v == '\n' );
}

template<typename T> class CXMLLoader
{
	public:
		~CXMLLoader()
		{
			T::Clear();
		}

		void Load( xmlNode *root, bool recursive, bool silent = true )
		{
			xmlNode *node = root;
			for( ;node ;node = node->next )
			{
				if( xmlStrcmp( T::GetName(), node->name ) )
				{
					Load( node->children, recursive, silent );
				}
				else
				{
					T *result = T::CreateInstance( node );
					if( result )
					{
						const xmlChar *id  = LoadStrAttr( node, _X"id" );
						const xmlChar *sid  = LoadStrAttr( node, _X"sid" );
						result->SetId( id );


						if( !silent )
						{
							printf( "%s %s\n", id, typeid( T ).name() );
						}
						T::Add( id, result );
						T::Addsid( sid, result );

					}

					if( recursive )
					{
						Load( node->children, recursive, silent  );
					}
				}
			}
		}
};

struct itbl
{
	int table[256];
	itbl()
	{
		for( int i = 0; i < 256; ++i )
		{
			char c = i;
			int res = -1;
			switch( c )
			{
					case('0'):res = 0;break;
					case('1'):res = 1;break;
					case('2'):res = 2;break;
					case('3'):res = 3;break;
					case('4'):res = 4;break;
					case('5'):res = 5;break;
					case('6'):res = 6;break;
					case('7'):res = 7;break;
					case('8'):res = 8;break;
					case('9'):res = 9;break;
			};
			table[i] = res;
		};
	};
};

inline int parse_fake_int( const xmlChar *&ptr )
{
	static itbl trans;
	char last = 0;
	while( trans.table[*ptr] ==-1 )
	{
		last = *ptr;
		++ptr;
	}
	int res = 0, curr;
	while(1)
	{
		curr = trans.table[*ptr];
		if( curr == -1 )return last == '-' ? -res - 1 : res;
		++ptr;
		res = res * 10 + curr;
	}
};

inline int parse_int( const xmlChar *&ptr )
{
	int res = parse_fake_int( ptr );
	return res < 0 ? res + 1 : res;
};

inline float parse_flt( const xmlChar *&ptr )
{
	static itbl trans;
	int res = 0, curr;
	float f = 1.0f;
	while(1)
	{
		curr = trans.table[*ptr];
		if( curr == -1 )return res * f;
		++ptr;
		res = res * 10 + curr;
		f *= 0.1f;
	}
};

inline float parse_float( const xmlChar *&ptr )
{
	float end = 0.0f;
	int beg =   parse_fake_int( ptr );
	if( *ptr == '.' )end = parse_flt( ++ptr );
	float f = beg < 0 ? beg - end + 1.0f: beg + end;
	return f;
};

inline void flush_str( const xmlChar *&ptr )
{
	while( ( *ptr == '\n' ) || ( *ptr == ' ' ) )
	{
		xmlChar *fuck = const_cast<xmlChar *>( ptr );
		*fuck = 0;
		ptr++;
	}
}

inline void parse_str( const xmlChar *&ptr )
{
	while( *(++ptr) != '\n' );
	//return res < 0 ? res + 1 : res;
};

template< class T > std::map<SStrng, T*> CAutoContainer<T>::container;
template< class T > std::map<SStrng, T*> CAutoContainer<T>::containersid;
template< class T > std::vector<T*>      CAutoContainer<T>::arrayContainer;


