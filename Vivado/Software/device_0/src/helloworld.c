/***************************** Include Files *********************************/
#include <stdio.h>
#include "xemaclite_example.h"
#include "xintc.h"
#include "xil_exception.h"
#include "xil_io.h"
#include "xil_printf.h"
#include "xil_cache.h"
#include "xgpio.h"

/************************** Constant Definitions *****************************/
#define GPIO_DEVICE_ID		XPAR_GPIO_0_DEVICE_ID
#define GPIO_CHANNEL1		1

#define INTC_DEVICE_ID		XPAR_INTC_0_DEVICE_ID

#define INTC_GPIO_INTERRUPT_ID	XPAR_INTC_0_GPIO_0_VEC_ID
#define INTC_EMACLITE_ID	XPAR_INTC_0_EMACLITE_0_VEC_ID

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

int EmacLitePhyLoopbackExample();

static int SendFrame(XEmacLite *InstancePtr, u32 PayloadSize);
static int SendAudioFrame(XEmacLite *InstancePtr, u32 PayloadSize);
static int RecvAudioFrame(u32 PayloadSize);
static int EmacLiteRecvFrame(u32 PayloadSize);
static void EmacLiteRecvHandler(void *CallBackRef);
static void EmacLiteSendHandler(void *CallBackRef);
static void EmacLiteDisableIntrSystem(XIntc *IntcInstancePtr,
							 u16 EmacLiteIntrId);
static int EmacLiteSetupIntrSystem();

void GpioHandler(void *CallBackRef);
int GpioIntrExample();
int GpioSetupIntrSystem();
void FPGAloop();
/************************** Variable Definitions *****************************/

XIntc IntcInstance;		/* Instance of the Interrupt Controller */
XGpio Gpio;			 /* The Instance of the GPIO Driver */
XEmacLite Emac ;
/*
 * Set up valid local MAC addresses. This loop back test uses the LocalAddress
 * both as a source and destination MAC address.
 */
static u8 LocalAddress[XEL_MAC_ADDR_SIZE] =
{
	0x00, 0x0A, 0x35, 0x01, 0x02, 0x03
};
static u8 Device1Address[XEL_MAC_ADDR_SIZE] =
{
	0x00, 0x0A, 0x35, 0x01, 0x02, 0x04
};

unsigned int ch1 ;
int data_arr[256];
int payload = 512 ;
u32 tx_payload[512];
u32 rx_payload[512];
unsigned int ptr ;
int offset ;
int main()
{

	Xil_ICacheEnable();
	//Xil_DCacheEnable();
	offset = 64 ;
	int i ;
	/*
	 * Run the EmacLite PHY loop back example , specify the Device ID
	 * that is generated in xparameters.h.
	 */
	print("Device 0 Online\n\r");
	XGpio_Initialize(&Gpio, GPIO_DEVICE_ID);
	GpioSetupIntrSystem();
	EmacLitePhyLoopbackExample();
	//FPGAloop();


	while(1){
		RecvFrameLength = 0 ;
		if(ch1 == 1){
			ch1 = 0 ;
			for(i = 0 ; i < 128 ; i ++){
				tx_payload[i] = Xil_In32(0xC0000000 + i*4);
				//Xil_Out32(0xC2000000  + (512 + i*4 + 20)%1024,rx_payload[i]);
			}
			Xil_DCacheEnable();
			SendAudioFrame(&Emac, payload);
			//while ((TransmitComplete == FALSE) && (RecvFrameLength == 0));
			RecvAudioFrame(payload);
			Xil_DCacheDisable();
			//for(i = 0 ; i < 128 ; i ++){
			//	Xil_Out32(0xC2000000  + 512 + i*4 ,rx_payload[i]);
			//}
		} else if(ch1 == 2){
			ch1 = 0 ;
			for(i = 0 ; i < 128 ; i ++){
				tx_payload[i] = Xil_In32(0xC0000000 + i*4 + 512  );
				//Xil_Out32(0xC2000000  + i*4+20,rx_payload[i]);
			}
			Xil_DCacheEnable();
			SendAudioFrame(&Emac, payload);
			//while ((TransmitComplete == FALSE) && (RecvFrameLength == 0));
			RecvAudioFrame(payload);
			Xil_DCacheDisable();
			//for(i = 0 ; i < 128 ; i ++){
			//	Xil_Out32(0xC2000000  + i*4,rx_payload[i]);
			//}
		}
	}


	while(1);

}

void FPGAloop(){
	int i ;
	while(1){
		RecvFrameLength = 0 ;
		if(ch1 == 1){
			ch1 = 0 ;
			for(i = 0 ; i < 128 ; i ++){
				tx_payload[i] = Xil_In32(0xC0000000 + i*4);

			}
			for(i = 0 ; i < 128 ; i ++){
				Xil_Out32(0xC2000000  + (512 + i*4 + 80)%1024 ,tx_payload[i]);
			}
		} else if(ch1 == 2){
			ch1 = 0 ;
			for(i = 0 ; i < 128 ; i ++){
				tx_payload[i] = Xil_In32(0xC0000000 + i*4 + 512  );

			}
			for(i = 0 ; i < 128 ; i ++){
				Xil_Out32(0xC2000000 + 80 + i*4,tx_payload[i]);
			}
		}
	}
}
static int SendAudioFrame(XEmacLite *XEmacInstancePtr, u32 PayloadSize)
{
	u8 *FramePtr;
	int Index;
	int Status;

	/*
	 * Set the Complete flag to false.
	 */
	TransmitComplete = FALSE;

	/*
	 * Assemble the frame with a destination address and the source address.
	 */
	FramePtr = (u8 *)TxFrame;
	*FramePtr++ = Device1Address[0];
	*FramePtr++ = Device1Address[1];
	*FramePtr++ = Device1Address[2];
	*FramePtr++ = Device1Address[3];
	*FramePtr++ = Device1Address[4];
	*FramePtr++ = Device1Address[5];

	/*
	 * Fill in the source MAC address.
	 */
	*FramePtr++ = LocalAddress[0];
	*FramePtr++ = LocalAddress[1];
	*FramePtr++ = LocalAddress[2];
	*FramePtr++ = LocalAddress[3];
	*FramePtr++ = LocalAddress[4];
	*FramePtr++ = LocalAddress[5];

	/*
	 * Set up the type/length field - be sure its in network order.
	 */
    *((u16 *)FramePtr) = Xil_Htons(PayloadSize);
    FramePtr++;
	FramePtr++;

	/*
	 * Now fill in the data field with known values so we can verify them
	 * on receive.
	 */
	for (Index = 0 ; Index <PayloadSize/4 ; Index ++){
		*FramePtr++ = (u8)(tx_payload[Index]&0x000000FF);
		*FramePtr++ = (u8)((tx_payload[Index]&0x0000FF00)>>8);
		*FramePtr++ = (u8)((tx_payload[Index]&0x00FF0000)>>16);
		*FramePtr++ = (u8)((tx_payload[Index]&0xFF000000)>>24);
		}

	/*
	 * Now send the frame.
	 */
	Status = XEmacLite_Send(XEmacInstancePtr, (u8 *)TxFrame,
			    PayloadSize + XEL_HEADER_SIZE);

	return  Status;
}

static int RecvAudioFrame(u32 PayloadSize)
{
	u8 *FramePtr;
	unsigned int data ;
	unsigned int b1,b2,b3,b4 ;
	int i ;
	/*
	 * This assumes MAC does not strip padding or CRC.
	 */
	if (RecvFrameLength != 0) {
		int Index;

		/*
		 * Verify length, which should be the payload size.
		 */
		if ((RecvFrameLength - (XEL_HEADER_SIZE + XEL_FCS_SIZE)) !=
				PayloadSize) {
			return XST_LOOPBACK_ERROR;
		}

		/*
		 * Verify the contents of the Received Frame.
		 */
		FramePtr = (u8 *)RxFrame;
		FramePtr += XEL_HEADER_SIZE;	/* Get past the header */
		i = 0 ;
		for (Index = 0; Index < PayloadSize/4; Index++) {
			b1 = *FramePtr++  ;
			b2 = (*FramePtr++)<<8 ;
			b3 = (*FramePtr++)<<16  ;
			b4 = (*FramePtr++)<<24  ;

			data =  (b4|b3|b2|b1) ;
			rx_payload[i] = data ;
			i++ ;
		}
	}

	return XST_SUCCESS;
}

int GpioSetupIntrSystem()
{
	/*
	 * Initialize the interrupt controller driver so that it's ready to use.
	 * specify the device ID that was generated in xparameters.h
	 */


	XIntc_Initialize(&IntcInstance, INTC_DEVICE_ID);

	/* Hook up interrupt service routine */
	XIntc_Connect(&IntcInstance, INTC_GPIO_INTERRUPT_ID, (Xil_ExceptionHandler)GpioHandler, &Gpio);

	/* Enable the interrupt vector at the interrupt controller */
	XIntc_Enable(&IntcInstance, INTC_GPIO_INTERRUPT_ID);

	/*
	 * Start the interrupt controller such that interrupts are recognized
	 * and handled by the processor
	 */
	XIntc_Start(&IntcInstance, XIN_REAL_MODE);

	/*
	 * Enable the GPIO channel interrupts so that push button can be
	 * detected and enable interrupts for the GPIO device
	 */
	XGpio_InterruptEnable(&Gpio, GPIO_CHANNEL1);
	XGpio_InterruptGlobalEnable(&Gpio);

	/*
	 * Initialize the exception table and register the interrupt
	 * controller handler with the exception table
	 */
	Xil_ExceptionInit();

	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_INT, (Xil_ExceptionHandler)XIntc_InterruptHandler, &IntcInstance);

	/* Enable non-critical exceptions */
	Xil_ExceptionEnable();

	return 0;
}

void GpioHandler(void *CallbackRef)
{
	XGpio *GpioPtr = (XGpio *)CallbackRef;

	ch1 = XGpio_DiscreteRead(&Gpio,1);
	/* Clear the Interrupt */
	XGpio_InterruptClear(GpioPtr, GPIO_CHANNEL1);

}

int EmacLitePhyLoopbackExample()
{
	u32 PhyAddress = 0;
	u32 TxLength;
	XEmacLite_Config *ConfigPtr;

	RecvFrameLength = 0;

	//Initialize the EmacLite device.
	ConfigPtr = XEmacLite_LookupConfig(EMAC_DEVICE_ID);
	XEmacLite_CfgInitialize(&Emac, ConfigPtr, ConfigPtr->BaseAddress);

	// Set the MAC address.
	XEmacLite_SetMacAddress(&Emac, LocalAddress);

	 // Empty any existing receive frames.
	XEmacLite_FlushReceive(&Emac);

	 //Set up the interrupt infrastructure.
	EmacLiteSetupIntrSystem();

	// Setup the EmacLite handlers.
	XEmacLite_SetRecvHandler((&Emac), (void *)(&Emac), (XEmacLite_Handler)EmacLiteRecvHandler);
	XEmacLite_SetSendHandler((&Emac), (void *)(&Emac), (XEmacLite_Handler)EmacLiteSendHandler);

	 //Enable the EmacLite interrupts.
	XEmacLite_EnableInterrupts(&Emac);

	 // Detect the PHY device and enable the MAC Loop back in the PHY.
	PhyAddress = EmacLitePhyDetect(&Emac);
	//EmacLiteEnablePhyLoopBack(&Emac, PhyAddress);

	//Send/Receive frames of varying sizes and verify the data in the received frames.
	//for (TxLength = 1; TxLength <= XEL_MTU_SIZE; ) {
	//	RecvFrameLength = 0;
	//	SendFrame(&Emac, TxLength);
	//	while ((TransmitComplete == FALSE) && (RecvFrameLength == 0));
	//	EmacLiteRecvFrame(TxLength++);
	//}

	//Disable the MAC Loop back in the PHY and disable/disconnect the EmacLite Interrupts.
	//EmacLiteDisablePhyLoopBack(&Emac, PhyAddress);
	//XEmacLite_DisableInterrupts(&Emac);
	//EmacLiteDisableIntrSystem(&IntcInstance, INTC_EMACLITE_ID);

	return XST_SUCCESS;
}


static int SendFrame(XEmacLite *XEmacInstancePtr, u32 PayloadSize)
{
	u8 *FramePtr;
	int Index;
	int Status;

	/*
	 * Set the Complete flag to false.
	 */
	TransmitComplete = FALSE;

	/*
	 * Assemble the frame with a destination address and the source address.
	 */
	FramePtr = (u8 *)TxFrame;
	*FramePtr++ = LocalAddress[0];
	*FramePtr++ = LocalAddress[1];
	*FramePtr++ = LocalAddress[2];
	*FramePtr++ = LocalAddress[3];
	*FramePtr++ = LocalAddress[4];
	*FramePtr++ = LocalAddress[5];

	/*
	 * Fill in the source MAC address.
	 */
	*FramePtr++ = LocalAddress[0];
	*FramePtr++ = LocalAddress[1];
	*FramePtr++ = LocalAddress[2];
	*FramePtr++ = LocalAddress[3];
	*FramePtr++ = LocalAddress[4];
	*FramePtr++ = LocalAddress[5];

	/*
	 * Set up the type/length field - be sure its in network order.
	 */
    *((u16 *)FramePtr) = Xil_Htons(PayloadSize);
    FramePtr++;
	FramePtr++;

	/*
	 * Now fill in the data field with known values so we can verify them
	 * on receive.
	 */
	for (Index = 0; Index < PayloadSize; Index++) {
		*FramePtr++ = (u8)Index;
	}

	/*
	 * Now send the frame.
	 */
	Status = XEmacLite_Send(XEmacInstancePtr, (u8 *)TxFrame,
			    PayloadSize + XEL_HEADER_SIZE);

	return  Status;
}


static int EmacLiteRecvFrame(u32 PayloadSize)
{
	u8 *FramePtr;
	//This assumes MAC does not strip padding or CRC.
	if (RecvFrameLength != 0) {
		int Index;

		// Verify length, which should be the payload size.
		if ((RecvFrameLength - (XEL_HEADER_SIZE + XEL_FCS_SIZE)) !=
				PayloadSize) {
			return XST_LOOPBACK_ERROR;
		}

		 //Verify the contents of the Received Frame.
		FramePtr = (u8 *)RxFrame;
		FramePtr += XEL_HEADER_SIZE;	/* Get past the header */

		for (Index = 0; Index < PayloadSize; Index++) {
			if (*FramePtr++ != (u8)Index) {
				return XST_LOOPBACK_ERROR;
			}
		}
	}

	return XST_SUCCESS;
}



static void EmacLiteRecvHandler(void *CallBackRef)
{
	XEmacLite *XEmacInstancePtr;
	//Convert the argument to something useful.
	XEmacInstancePtr = (XEmacLite *)CallBackRef;
	//Handle the Receive callback.
	RecvFrameLength = XEmacLite_Recv(XEmacInstancePtr, (u8 *)RxFrame);

}


static void EmacLiteSendHandler(void *CallBackRef)
{
	XEmacLite *XEmacInstancePtr;
	//Convert the argument to something useful.
	XEmacInstancePtr = (XEmacLite *)CallBackRef;
	//Handle the Transmit callback.
	TransmitComplete = TRUE;

}


static int EmacLiteSetupIntrSystem()
{
	//Initialize the interrupt controller driver so that it is ready to use.
	XIntc_Initialize(&IntcInstance, INTC_DEVICE_ID);

	/*
	 * Connect a device driver handler that will be called when an interrupt
	 * for the device occurs, the device driver handler performs the
	 * specific interrupt processing for the device.
	 */
	XIntc_Connect(&IntcInstance, INTC_EMACLITE_ID, XEmacLite_InterruptHandler, (void *)(&Emac));
	XIntc_Connect(&IntcInstance, INTC_GPIO_INTERRUPT_ID, (Xil_ExceptionHandler)GpioHandler, &Gpio);//

	/*
	 * Start the interrupt controller such that interrupts are enabled for
	 * all devices that cause interrupts, specific real mode so that
	 * the EmacLite can cause interrupts thru the interrupt controller.
	 */
	XIntc_Start(&IntcInstance, XIN_REAL_MODE);

	// Enable the interrupt for the EmacLite in the Interrupt controller.
	XIntc_Enable(&IntcInstance, INTC_EMACLITE_ID);
	XIntc_Enable(&IntcInstance, INTC_GPIO_INTERRUPT_ID); //

	XGpio_InterruptEnable(&Gpio, GPIO_CHANNEL1); //
	XGpio_InterruptGlobalEnable(&Gpio); //


	// Initialize the exception table.
	Xil_ExceptionInit();

	//Register the interrupt controller handler with the exception table.
	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_INT, (Xil_ExceptionHandler) XIntc_InterruptHandler,&IntcInstance);

	//Enable non-critical exceptions.
	Xil_ExceptionEnable();


	return XST_SUCCESS;
}


static void EmacLiteDisableIntrSystem(XIntc *IntcInstancePtr,
					 u16 EmacLiteIntrId)
{
    //Disconnect and disable the interrupts for the EmacLite device.
	XIntc_Disconnect(&IntcInstance, INTC_EMACLITE_ID);

}
