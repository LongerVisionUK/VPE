# Copyright 2018 VeriSilicon Inc. All Rights Reserved.

ifeq ($(obj),)
obj = .
endif

ifeq ($(EMULATOR),y)
EXTRA_CFLAGS += -DEMULATOR
endif

transcoder_pcie-objs += transcoder.o pcie.o edma.o vc8000e.o bigsea.o vc8000d.o
transcoder_pcie-objs += memory.o interrupt.o debug_trace.o hw_monitor.o
transcoder_pcie-objs += encoder.o misc_ip.o

obj-m += transcoder_pcie.o

KERN_DIR := /lib/modules/$(shell uname -r)/build


all:
	$(MAKE) -C $(KERN_DIR) M=`pwd` modules

test:
	gcc -o edma_test_hugepage edma_test_hugepage.c -lhugetlbfs
	gcc -o edma_link_test edma_link_test.c -lhugetlbfs
	gcc -o mem_test mem_test.c
.PHONY: clean
clean:
	make -C $(KERN_DIR) M=`pwd` clean
	rm -rf edma_test_hugepage mem_test edma_link_test
