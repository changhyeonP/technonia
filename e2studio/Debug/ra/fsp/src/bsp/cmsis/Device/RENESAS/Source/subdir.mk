################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../ra/fsp/src/bsp/cmsis/Device/RENESAS/Source/startup.c \
../ra/fsp/src/bsp/cmsis/Device/RENESAS/Source/system.c 

C_DEPS += \
./ra/fsp/src/bsp/cmsis/Device/RENESAS/Source/startup.d \
./ra/fsp/src/bsp/cmsis/Device/RENESAS/Source/system.d 

CREF += \
icu_ek_ra8p1_ep.cref 

OBJS += \
./ra/fsp/src/bsp/cmsis/Device/RENESAS/Source/startup.o \
./ra/fsp/src/bsp/cmsis/Device/RENESAS/Source/system.o 

MAP += \
icu_ek_ra8p1_ep.map 


# Each subdirectory must supply rules for building sources it contributes
ra/fsp/src/bsp/cmsis/Device/RENESAS/Source/%.o: ../ra/fsp/src/bsp/cmsis/Device/RENESAS/Source/%.c
	@echo 'Building file: $<'
	$(file > $@.in,-mcpu=cortex-m85 -mthumb -mlittle-endian -mfloat-abi=hard -O2 -ffunction-sections -fdata-sections -fmessage-length=0 -funsigned-char -Wunused -Wuninitialized -Wall -Wextra -Wmissing-declarations -Wconversion -Wpointer-arith -Wshadow -Waggregate-return -Wno-parentheses-equality -Wfloat-equal -gdwarf-4 -g3 -std=c99 -flax-vector-conversions -fshort-enums -fno-unroll-loops -I"C:\\Users\\qnfmd\\ra8p1_example\\ek_ra8p1\\icu\\icu_ek_ra8p1_ep\\e2studio\\src" -I"." -I"C:\\Users\\qnfmd\\ra8p1_example\\ek_ra8p1\\icu\\icu_ek_ra8p1_ep\\e2studio\\ra\\fsp\\inc" -I"C:\\Users\\qnfmd\\ra8p1_example\\ek_ra8p1\\icu\\icu_ek_ra8p1_ep\\e2studio\\ra\\fsp\\inc\\api" -I"C:\\Users\\qnfmd\\ra8p1_example\\ek_ra8p1\\icu\\icu_ek_ra8p1_ep\\e2studio\\ra\\fsp\\inc\\instances" -I"C:\\Users\\qnfmd\\ra8p1_example\\ek_ra8p1\\icu\\icu_ek_ra8p1_ep\\e2studio\\ra\\arm\\CMSIS_6\\CMSIS\\Core\\Include" -I"C:\\Users\\qnfmd\\ra8p1_example\\ek_ra8p1\\icu\\icu_ek_ra8p1_ep\\e2studio\\ra_gen" -I"C:\\Users\\qnfmd\\ra8p1_example\\ek_ra8p1\\icu\\icu_ek_ra8p1_ep\\e2studio\\ra_cfg\\fsp_cfg\\bsp" -I"C:\\Users\\qnfmd\\ra8p1_example\\ek_ra8p1\\icu\\icu_ek_ra8p1_ep\\e2studio\\ra_cfg\\fsp_cfg" -D_RENESAS_RA_ -D_RA_CORE=CPU0 -D_RA_ORDINAL=1 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -x c "$<" -c -o "$@")
	@clang --target=arm-none-eabi @"$@.in"

