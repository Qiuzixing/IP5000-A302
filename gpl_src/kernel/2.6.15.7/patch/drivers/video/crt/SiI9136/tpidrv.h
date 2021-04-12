// functions in the driver called by the tx component
void    SiiMhlTxDrvTmdsControl( bool_t enable,bool_t UnMute );

#ifdef MHL_CONNECTION_STATE_MACHINE //(
bool_t  SiiMhlTxDrvCBusBusy(void);
bool_t  SiiMhlTxDrvSendCbusCommand ( cbus_req_t *pReq  );
void    SiiMhlTxDrvPowBitChange( bool_t powerOn );
#else //)(
#define SiiMhlTxTmdsEnable(UnMute)  SiiMhlTxDrvTmdsControl(true,UnMute);TPI_DEBUG_PRINT((TPI_DEBUG_CHANNEL,"SiiMhlTxTmdsEnable\n"));
#endif //)

// Functions in the tx component called by the driver
void	SiiMhlTxMscCommandDone( uint8_t data1 );
///////////////////////////////////////////////////////////////////////////////
//
// SiiMhlTxGotMhlIntr
//
// This function is called by the driver to inform of arrival of a MHL INTERRUPT.
//
// It is called in interrupt context to meet some MHL specified timings, therefore,
// it should not have to call app layer and do negligible processing, no printfs.
//
extern	void	SiiMhlTxGotMhlIntr( uint8_t intr_0, uint8_t intr_1 );
///////////////////////////////////////////////////////////////////////////////
//
// SiiMhlTxGotMhlStatus
//
// This function is called by the driver to inform of arrival of a MHL STATUS.
//
// It is called in interrupt context to meet some MHL specified timings, therefore,
// it should not have to call app layer and do negligible processing, no printfs.
//
extern	void	SiiMhlTxGotMhlStatus( uint8_t status_0, uint8_t status_1 );
///////////////////////////////////////////////////////////////////////////////
//
// SiiMhlTxGotMhlMscMsg
//
// This function is called by the driver to inform of arrival of a MHL STATUS.
//
// It is called in interrupt context to meet some MHL specified timings, therefore,
// it should not have to call app layer and do negligible processing, no printfs.
//
// Application shall not call this function.
//
extern	void	SiiMhlTxGotMhlMscMsg( uint8_t subCommand, uint8_t cmdData );

///////////////////////////////////////////////////////////////////////////////
//
// SiiMhlTxNotifyDsHpdChange
//
// Informs MhlTx component of a Downstream HPD change (when h/w receives
// SET_HPD or CLEAR_HPD).
//
extern	void	SiiMhlTxNotifyDsHpdChange( uint8_t dsHpdStatus );

///////////////////////////////////////////////////////////////////////////////
//
// SiiMhlTxNotifyConnection
//
// This function is called by the driver to inform of connection status change.
//
// It is called in interrupt context to meet some MHL specified timings, therefore,
// it should not have to call app layer and do negligible processing, no printfs.
//
extern	void	SiiMhlTxNotifyConnection( bool_t mhlConnected );


#define       MHL_LOGICAL_DEVICE_MAP          (MHL_DEV_LD_AUDIO | MHL_DEV_LD_VIDEO | MHL_DEV_LD_MEDIA | MHL_DEV_LD_GUI )
#define SILICON_IMAGE_ADOPTER_ID 322
