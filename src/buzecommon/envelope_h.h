

/* this ALWAYS GENERATED file contains the definitions for the interfaces */


 /* File created by MIDL compiler version 6.00.0361 */
/* at Sun Apr 30 15:53:34 2006
 */
/* Compiler settings for .\envelope.odl:
    Oicf, W1, Zp8, env=Win32 (32b run)
    protocol : dce , ms_ext, c_ext, robust
    error checks: allocation ref bounds_check enum stub_data 
    VC __declspec() decoration level: 
         __declspec(uuid()), __declspec(selectany), __declspec(novtable)
         DECLSPEC_UUID(), MIDL_INTERFACE()
*/
//@@MIDL_FILE_HEADING(  )

#pragma warning( disable: 4049 )  /* more than 64k source lines */


/* verify that the <rpcndr.h> version is high enough to compile this file*/
#ifndef __REQUIRED_RPCNDR_H_VERSION__
#define __REQUIRED_RPCNDR_H_VERSION__ 475
#endif

#include "rpc.h"
#include "rpcndr.h"

#ifndef __RPCNDR_H_VERSION__
#error this stub requires an updated version of <rpcndr.h>
#endif // __RPCNDR_H_VERSION__


#ifndef __envelope_h_h__
#define __envelope_h_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

/* Forward Declarations */ 

#ifndef ___DEnvelope_FWD_DEFINED__
#define ___DEnvelope_FWD_DEFINED__
typedef interface _DEnvelope _DEnvelope;
#endif 	/* ___DEnvelope_FWD_DEFINED__ */


#ifndef ___DEnvelopeEvents_FWD_DEFINED__
#define ___DEnvelopeEvents_FWD_DEFINED__
typedef interface _DEnvelopeEvents _DEnvelopeEvents;
#endif 	/* ___DEnvelopeEvents_FWD_DEFINED__ */


#ifndef __Envelope_FWD_DEFINED__
#define __Envelope_FWD_DEFINED__

#ifdef __cplusplus
typedef class Envelope Envelope;
#else
typedef struct Envelope Envelope;
#endif /* __cplusplus */

#endif 	/* __Envelope_FWD_DEFINED__ */


#ifdef __cplusplus
extern "C"{
#endif 

void * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void * ); 


#ifndef __ENVELOPELib_LIBRARY_DEFINED__
#define __ENVELOPELib_LIBRARY_DEFINED__

/* library ENVELOPELib */
/* [control][helpstring][helpfile][version][uuid] */ 


DEFINE_GUID(LIBID_ENVELOPELib,0x396D2EB4,0x53AA,0x11D1,0xA1,0xD9,0x46,0x76,0xEE,0x00,0x00,0x00);

#ifndef ___DEnvelope_DISPINTERFACE_DEFINED__
#define ___DEnvelope_DISPINTERFACE_DEFINED__

/* dispinterface _DEnvelope */
/* [hidden][helpstring][uuid] */ 


DEFINE_GUID(DIID__DEnvelope,0x396D2EB5,0x53AA,0x11D1,0xA1,0xD9,0x46,0x76,0xEE,0x00,0x00,0x00);

#if defined(__cplusplus) && !defined(CINTERFACE)

    MIDL_INTERFACE("396D2EB5-53AA-11D1-A1D9-4676EE000000")
    _DEnvelope : public IDispatch
    {
    };
    
#else 	/* C style interface */

    typedef struct _DEnvelopeVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            _DEnvelope * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            _DEnvelope * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            _DEnvelope * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            _DEnvelope * This,
            /* [out] */ UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            _DEnvelope * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            _DEnvelope * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR *rgszNames,
            /* [in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            _DEnvelope * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS *pDispParams,
            /* [out] */ VARIANT *pVarResult,
            /* [out] */ EXCEPINFO *pExcepInfo,
            /* [out] */ UINT *puArgErr);
        
        END_INTERFACE
    } _DEnvelopeVtbl;

    interface _DEnvelope
    {
        CONST_VTBL struct _DEnvelopeVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define _DEnvelope_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define _DEnvelope_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define _DEnvelope_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define _DEnvelope_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define _DEnvelope_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define _DEnvelope_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define _DEnvelope_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)

#endif /* COBJMACROS */


#endif 	/* C style interface */


#endif 	/* ___DEnvelope_DISPINTERFACE_DEFINED__ */


#ifndef ___DEnvelopeEvents_DISPINTERFACE_DEFINED__
#define ___DEnvelopeEvents_DISPINTERFACE_DEFINED__

/* dispinterface _DEnvelopeEvents */
/* [helpstring][uuid] */ 


DEFINE_GUID(DIID__DEnvelopeEvents,0x396D2EB6,0x53AA,0x11D1,0xA1,0xD9,0x46,0x76,0xEE,0x00,0x00,0x00);

#if defined(__cplusplus) && !defined(CINTERFACE)

    MIDL_INTERFACE("396D2EB6-53AA-11D1-A1D9-4676EE000000")
    _DEnvelopeEvents : public IDispatch
    {
    };
    
#else 	/* C style interface */

    typedef struct _DEnvelopeEventsVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            _DEnvelopeEvents * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            _DEnvelopeEvents * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            _DEnvelopeEvents * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            _DEnvelopeEvents * This,
            /* [out] */ UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            _DEnvelopeEvents * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            _DEnvelopeEvents * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR *rgszNames,
            /* [in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            _DEnvelopeEvents * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS *pDispParams,
            /* [out] */ VARIANT *pVarResult,
            /* [out] */ EXCEPINFO *pExcepInfo,
            /* [out] */ UINT *puArgErr);
        
        END_INTERFACE
    } _DEnvelopeEventsVtbl;

    interface _DEnvelopeEvents
    {
        CONST_VTBL struct _DEnvelopeEventsVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define _DEnvelopeEvents_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define _DEnvelopeEvents_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define _DEnvelopeEvents_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define _DEnvelopeEvents_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define _DEnvelopeEvents_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define _DEnvelopeEvents_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define _DEnvelopeEvents_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)

#endif /* COBJMACROS */


#endif 	/* C style interface */


#endif 	/* ___DEnvelopeEvents_DISPINTERFACE_DEFINED__ */


DEFINE_GUID(CLSID_Envelope,0x396D2EB7,0x53AA,0x11D1,0xA1,0xD9,0x46,0x76,0xEE,0x00,0x00,0x00);

#ifdef __cplusplus

class DECLSPEC_UUID("396D2EB7-53AA-11D1-A1D9-4676EE000000")
Envelope;
#endif
#endif /* __ENVELOPELib_LIBRARY_DEFINED__ */

/* Additional Prototypes for ALL interfaces */

/* end of Additional Prototypes */

#ifdef __cplusplus
}
#endif

#endif


