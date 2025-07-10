#pragma once
// minified source sdk
namespace ssdk {
    namespace dt {
	    struct SendTable;
		struct SendProp // sizeof=0x50
		{                                       // XREF: _Z17SendPropUtlVectorPciiPFvPviiEi8SendPropPFS0_PKS3_PKvS7_P20CSendProxyRecipientsiE/r
			enum flags : __int32
			{                                       // XREF: DVariant/r
			    DPT_Int              = 0x0,
			    DPT_Float            = 0x1,
			    DPT_Vector           = 0x2,
			    DPT_VectorXY         = 0x3,
			    DPT_String           = 0x4,
			    DPT_Array            = 0x5,
			    DPT_DataTable        = 0x6,
			    DPT_Int64            = 0x7,
			    DPT_NUMSendPropTypes = 0x8,
			};

			typedef int (*ArrayLengthSendProxyFn)(const void *, int);
			typedef void (*SendVarProxyFn)(const SendProp *, const void *, const void *, void *, int, int);
			typedef void *(*SendTableProxyFn)(const SendProp *, const void *, const void *, void *, int);
			int (**_vptr_SendProp)(...);        // XREF: __tcf_0+13/o
												// __tcf_0+1C/r ...
			void *m_pMatchingRecvProp;      // XREF: SendPropUtlVector(char *,int,int,void (*)(void *,int,int),int,SendProp,void * (*)(SendProp const*,void const*,void const*,CSendProxyRecipients *,int))+1CD/r
												// SendPropUtlVector(char *,int,int,void (*)(void *,int,int),int,SendProp,void * (*)(SendProp const*,void const*,void const*,CSendProxyRecipients *,int))+2F6/r
			SendProp::flags m_Type;
												// XREF: SendPropUtlVector(char *,int,int,void (*)(void *,int,int),int,SendProp,void * (*)(SendProp const*,void const*,void const*,CSendProxyRecipients *,int))+1D3/r
												// SendPropUtlVector(char *,int,int,void (*)(void *,int,int),int,SendProp,void * (*)(SendProp const*,void const*,void const*,CSendProxyRecipients *,int))+305/r
			int m_nBits;                        // XREF: SendPropUtlVector(char *,int,int,void (*)(void *,int,int),int,SendProp,void * (*)(SendProp const*,void const*,void const*,CSendProxyRecipients *,int))+1D9/r
												// SendPropUtlVector(char *,int,int,void (*)(void *,int,int),int,SendProp,void * (*)(SendProp const*,void const*,void const*,CSendProxyRecipients *,int))+30E/r
			float m_fLowValue;                  // XREF: SendPropUtlVector(char *,int,int,void (*)(void *,int,int),int,SendProp,void * (*)(SendProp const*,void const*,void const*,CSendProxyRecipients *,int))+1DF/r
												// SendPropUtlVector(char *,int,int,void (*)(void *,int,int),int,SendProp,void * (*)(SendProp const*,void const*,void const*,CSendProxyRecipients *,int))+317/r
			float m_fHighValue;                 // XREF: SendPropUtlVector(char *,int,int,void (*)(void *,int,int),int,SendProp,void * (*)(SendProp const*,void const*,void const*,CSendProxyRecipients *,int))+1E5/r
												// SendPropUtlVector(char *,int,int,void (*)(void *,int,int),int,SendProp,void * (*)(SendProp const*,void const*,void const*,CSendProxyRecipients *,int))+320/r
			SendProp *m_pArrayProp;             // XREF: SendPropUtlVector(char *,int,int,void (*)(void *,int,int),int,SendProp,void * (*)(SendProp const*,void const*,void const*,CSendProxyRecipients *,int))+1EB/r
												// SendPropUtlVector(char *,int,int,void (*)(void *,int,int),int,SendProp,void * (*)(SendProp const*,void const*,void const*,CSendProxyRecipients *,int))+329/r
			ArrayLengthSendProxyFn m_ArrayLengthProxy;
												// XREF: SendPropUtlVector(char *,int,int,void (*)(void *,int,int),int,SendProp,void * (*)(SendProp const*,void const*,void const*,CSendProxyRecipients *,int))+1F1/r
												// SendPropUtlVector(char *,int,int,void (*)(void *,int,int),int,SendProp,void * (*)(SendProp const*,void const*,void const*,CSendProxyRecipients *,int))+332/r
			int m_nElements;                    // XREF: SendPropUtlVector(char *,int,int,void (*)(void *,int,int),int,SendProp,void * (*)(SendProp const*,void const*,void const*,CSendProxyRecipients *,int))+1F7/r
												// SendPropUtlVector(char *,int,int,void (*)(void *,int,int),int,SendProp,void * (*)(SendProp const*,void const*,void const*,CSendProxyRecipients *,int))+33B/r
			int m_ElementStride;                // XREF: SendPropUtlVector(char *,int,int,void (*)(void *,int,int),int,SendProp,void * (*)(SendProp const*,void const*,void const*,CSendProxyRecipients *,int))+1FD/r
												// SendPropUtlVector(char *,int,int,void (*)(void *,int,int),int,SendProp,void * (*)(SendProp const*,void const*,void const*,CSendProxyRecipients *,int))+344/r
			char *m_pExcludeDTName;             // XREF: SendPropUtlVector(char *,int,int,void (*)(void *,int,int),int,SendProp,void * (*)(SendProp const*,void const*,void const*,CSendProxyRecipients *,int))+203/r
												// SendPropUtlVector(char *,int,int,void (*)(void *,int,int),int,SendProp,void * (*)(SendProp const*,void const*,void const*,CSendProxyRecipients *,int))+34D/r
			char *m_pParentArrayPropName;       // XREF: SendPropUtlVector(char *,int,int,void (*)(void *,int,int),int,SendProp,void * (*)(SendProp const*,void const*,void const*,CSendProxyRecipients *,int))+209/r
												// SendPropUtlVector(char *,int,int,void (*)(void *,int,int),int,SendProp,void * (*)(SendProp const*,void const*,void const*,CSendProxyRecipients *,int))+356/r
			char *m_pVarName;                   // XREF: SendPropUtlVector(char *,int,int,void (*)(void *,int,int),int,SendProp,void * (*)(SendProp const*,void const*,void const*,CSendProxyRecipients *,int))+20F/r
												// SendPropUtlVector(char *,int,int,void (*)(void *,int,int),int,SendProp,void * (*)(SendProp const*,void const*,void const*,CSendProxyRecipients *,int))+35F/r
			float m_fHighLowMul;                // XREF: SendPropUtlVector(char *,int,int,void (*)(void *,int,int),int,SendProp,void * (*)(SendProp const*,void const*,void const*,CSendProxyRecipients *,int))+215/r
												// SendPropUtlVector(char *,int,int,void (*)(void *,int,int),int,SendProp,void * (*)(SendProp const*,void const*,void const*,CSendProxyRecipients *,int))+368/r
			unsigned char m_priority;                   // XREF: SendPropUtlVector(char *,int,int,void (*)(void *,int,int),int,SendProp,void * (*)(SendProp const*,void const*,void const*,CSendProxyRecipients *,int))+21B/r
												// SendPropUtlVector(char *,int,int,void (*)(void *,int,int),int,SendProp,void * (*)(SendProp const*,void const*,void const*,CSendProxyRecipients *,int))+371/r
			// padding byte
			// padding byte
			// padding byte
			int m_Flags;                        // XREF: SendPropUtlVector(char *,int,int,void (*)(void *,int,int),int,SendProp,void * (*)(SendProp const*,void const*,void const*,CSendProxyRecipients *,int))+222/r
												// SendPropUtlVector(char *,int,int,void (*)(void *,int,int),int,SendProp,void * (*)(SendProp const*,void const*,void const*,CSendProxyRecipients *,int))+37B/r
			SendVarProxyFn m_ProxyFn;           // XREF: SendPropUtlVector(char *,int,int,void (*)(void *,int,int),int,SendProp,void * (*)(SendProp const*,void const*,void const*,CSendProxyRecipients *,int))+228/r
												// SendPropUtlVector(char *,int,int,void (*)(void *,int,int),int,SendProp,void * (*)(SendProp const*,void const*,void const*,CSendProxyRecipients *,int))+384/r
			SendTableProxyFn m_DataTableProxyFn;
												// XREF: SendPropUtlVector(char *,int,int,void (*)(void *,int,int),int,SendProp,void * (*)(SendProp const*,void const*,void const*,CSendProxyRecipients *,int))+22E/r
												// SendPropUtlVector(char *,int,int,void (*)(void *,int,int),int,SendProp,void * (*)(SendProp const*,void const*,void const*,CSendProxyRecipients *,int))+38A/r
			SendTable *m_pDataTable;            // XREF: SendPropUtlVector(char *,int,int,void (*)(void *,int,int),int,SendProp,void * (*)(SendProp const*,void const*,void const*,CSendProxyRecipients *,int))+234/r
												// SendPropUtlVector(char *,int,int,void (*)(void *,int,int),int,SendProp,void * (*)(SendProp const*,void const*,void const*,CSendProxyRecipients *,int))+390/r
			int m_Offset;                       // XREF: SendPropUtlVector(char *,int,int,void (*)(void *,int,int),int,SendProp,void * (*)(SendProp const*,void const*,void const*,CSendProxyRecipients *,int))+23A/r
												// SendPropUtlVector(char *,int,int,void (*)(void *,int,int),int,SendProp,void * (*)(SendProp const*,void const*,void const*,CSendProxyRecipients *,int))+396/r
			const void *m_pExtraData;           // XREF: SendPropUtlVector(char *,int,int,void (*)(void *,int,int),int,SendProp,void * (*)(SendProp const*,void const*,void const*,CSendProxyRecipients *,int))+240/r
												// SendPropUtlVector(char *,int,int,void (*)(void *,int,int),int,SendProp,void * (*)(SendProp const*,void const*,void const*,CSendProxyRecipients *,int))+39C/r
		};

		static_assert(sizeof(SendProp) == 0x54);

        struct SendTable {
            SendProp* m_pProps;
            int			m_nProps;
            char* m_pNetTableName;	// The name matched between client and server.
        };

        struct ServerClass {
            char* m_pNetworkName;
            SendTable* m_pTable;
            ServerClass* m_pNext;
        };
    }
}
