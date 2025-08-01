################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/SEGGER_RTT/SEGGER_RTT.c \
../src/SEGGER_RTT/SEGGER_RTT_printf.c 

C_DEPS += \
./src/SEGGER_RTT/SEGGER_RTT.d \
./src/SEGGER_RTT/SEGGER_RTT_printf.d 

CREF += \
min_codec.cref 

OBJS += \
./src/SEGGER_RTT/SEGGER_RTT.o \
./src/SEGGER_RTT/SEGGER_RTT_printf.o 

MAP += \
min_codec.map 


# Each subdirectory must supply rules for building sources it contributes
src/SEGGER_RTT/%.o: ../src/SEGGER_RTT/%.c
	@echo 'Building file: $<'
	$(file > $@.in,-mcpu=cortex-m85 -mthumb -mlittle-endian -mfloat-abi=hard -O2 -ffunction-sections -fdata-sections -fmessage-length=0 -funsigned-char -Wunused -Wuninitialized -Wall -Wextra -Wmissing-declarations -Wconversion -Wpointer-arith -Wshadow -Waggregate-return -Wno-parentheses-equality -Wfloat-equal -gdwarf-4 -g3 -std=c99 -flax-vector-conversions -fshort-enums -fno-unroll-loops -I"C:\\Users\\qnfmd\\ra8p1_example\\ek_ra8p1\\min_codec\\src" -I"." -I"C:\\Users\\qnfmd\\ra8p1_example\\ek_ra8p1\\min_codec\\ra\\fsp\\inc" -I"C:\\Users\\qnfmd\\ra8p1_example\\ek_ra8p1\\min_codec\\ra\\fsp\\inc\\api" -I"C:\\Users\\qnfmd\\ra8p1_example\\ek_ra8p1\\min_codec\\ra\\fsp\\inc\\instances" -I"C:\\Users\\qnfmd\\ra8p1_example\\ek_ra8p1\\min_codec\\ra\\arm\\CMSIS_6\\CMSIS\\Core\\Include" -I"C:\\Users\\qnfmd\\ra8p1_example\\ek_ra8p1\\min_codec\\ra_gen" -I"C:\\Users\\qnfmd\\ra8p1_example\\ek_ra8p1\\min_codec\\ra_cfg\\fsp_cfg\\bsp" -I"C:\\Users\\qnfmd\\ra8p1_example\\ek_ra8p1\\min_codec\\ra_cfg\\fsp_cfg" -D_RENESAS_RA_ -D_RA_CORE=CPU0 -D_RA_ORDINAL=1 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -x c "$<" -c -o "$@")
	@clang --target=arm-none-eabi @"$@.in"

