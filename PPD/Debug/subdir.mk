################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../funcionesDisco.c \
../listaPPD.c \
../mapeoArchivos.c \
../ppd.c \
../threadPPD.c 

OBJS += \
./funcionesDisco.o \
./listaPPD.o \
./mapeoArchivos.o \
./ppd.o \
./threadPPD.o 

C_DEPS += \
./funcionesDisco.d \
./listaPPD.d \
./mapeoArchivos.d \
./ppd.d \
./threadPPD.d 


# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -D__USE_LARGEFILE -D_LARGEFILE_SOURCE -D_FILE_OFFSET_BITS=64 -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


