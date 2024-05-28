#include "stdint.h"
#include "memio.h"
#include "Interrupt.h"
#include "HalInterrupt.h"
#include "armcpu.h"

extern volatile GicCput_t* GicCpu;
extern volatile GicDist_t* GicDist;

static InterHdlr_fptr sHanlders[INTERRUPT_HANDLER_NUM];

void Hal_interrupt_init(void)
{
    GicCpu->cpucontrol.bits.Enable = 1;
    GicCpu->prioritymask.bits.Prioritymask = GIC_PRIORITY_MASK_NONE;
    GicDist->distributorctrl.bits.Enable = 1;

    for(uint32_t i = 0; i< INTERRUPT_HANDLER_NUM; i++)
    {
        sHanlders[i] = NULL;
    }
    
    enable_irq();
}

void Hal_interrupt_enable(uint32_t interrupt_num)
{
    if(interrupt_num < GIC_IRQ_START || interrupt_num > GIC_IRQ_END)
    {
        return;
    }

    uint32_t bit_num = interrupt_num - GIC_IRQ_START;

    if(bit_num < GIC_IRQ_START)
    {
        SET_BIT(GicDist->setenable1, bit_num);
    }
    else
    {
        bit_num -= GIC_IRQ_START;
        SET_BIT(GicDist->setenable2, bit_num);
    }
}

 void Hal_interrupt_disable(uint32_t interrupt_num)
 {
    if(interrupt_num < GIC_IRQ_START || interrupt_num > GIC_IRQ_END)
    {
        return;
    }

    uint32_t bit_num = interrupt_num - GIC_IRQ_START;

    if(bit_num < GIC_IRQ_START)
    {
        CLR_BIT(GicDist->setenable1, bit_num);
    }
    else
    {
        bit_num -= GIC_IRQ_START;
        CLR_BIT(GicDist->setenable2, bit_num);
    }
 }

 void Hal_interrupt_register_handler(InterHdlr_fptr handler, uint32_t interrupt_num)
 {
    sHanlders[interrupt_num] = handler;
 }

 void Hal_interrupt_run_handler(void)
 {
    uint32_t interrupt_num = GicCpu->interruptack.bits.InterruptID; // 현재 하드웨어에서 대기 중인 인터럽트 IRQ

    if(sHanlders[interrupt_num] != NULL) // NULL이면 에러이므로 실행하지 않는다.
    {
        sHanlders[interrupt_num](); //인터럽트 핸들러 실행
    }

    GicCpu->endofinterrupt.bits.InterruptID = interrupt_num; //해당 인터럽트가 완료됐다. endofinterrupt bit 설정
 }