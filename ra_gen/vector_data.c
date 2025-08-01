/* generated vector source file - do not edit */
#include "bsp_api.h"
/* Do not build these data structures if no interrupts are currently allocated because IAR will have build errors. */
#if VECTOR_DATA_IRQ_COUNT > 0
        BSP_DONT_REMOVE const fsp_vector_t g_vector_table[BSP_ICU_VECTOR_NUM_ENTRIES] BSP_PLACE_IN_SECTION(BSP_SECTION_APPLICATION_VECTORS) =
        {
                        [0] = ssi_txi_isr, /* SSI0 TXI (Transmit data empty) */
            [1] = ssi_rxi_isr, /* SSI0 RXI (Receive data full) */
            [2] = ssi_int_isr, /* SSI0 INT (Error interrupt) */
            [3] = iic_master_rxi_isr, /* IIC1 RXI (Receive data full) */
            [4] = iic_master_txi_isr, /* IIC1 TXI (Transmit data empty) */
            [5] = iic_master_tei_isr, /* IIC1 TEI (Transmit end) */
            [6] = iic_master_eri_isr, /* IIC1 ERI (Transfer error) */
        };
        #if BSP_FEATURE_ICU_HAS_IELSR
        const bsp_interrupt_event_t g_interrupt_event_link_select[BSP_ICU_VECTOR_NUM_ENTRIES] =
        {
            [0] = BSP_PRV_VECT_ENUM(EVENT_SSI0_TXI,GROUP0), /* SSI0 TXI (Transmit data empty) */
            [1] = BSP_PRV_VECT_ENUM(EVENT_SSI0_RXI,GROUP1), /* SSI0 RXI (Receive data full) */
            [2] = BSP_PRV_VECT_ENUM(EVENT_SSI0_INT,GROUP2), /* SSI0 INT (Error interrupt) */
            [3] = BSP_PRV_VECT_ENUM(EVENT_IIC1_RXI,GROUP3), /* IIC1 RXI (Receive data full) */
            [4] = BSP_PRV_VECT_ENUM(EVENT_IIC1_TXI,GROUP4), /* IIC1 TXI (Transmit data empty) */
            [5] = BSP_PRV_VECT_ENUM(EVENT_IIC1_TEI,GROUP5), /* IIC1 TEI (Transmit end) */
            [6] = BSP_PRV_VECT_ENUM(EVENT_IIC1_ERI,GROUP6), /* IIC1 ERI (Transfer error) */
        };
        #endif
        #endif
