################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Comunes/librerias/collections/list.c \
../Comunes/librerias/collections/queue.c 

OBJS += \
./Comunes/librerias/collections/list.o \
./Comunes/librerias/collections/queue.o 

C_DEPS += \
./Comunes/librerias/collections/list.d \
./Comunes/librerias/collections/queue.d 


# Each subdirectory must supply rules for building sources it contributes
Comunes/librerias/collections/%.o: ../Comunes/librerias/collections/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -D__USE_LARGEFILE -D_LARGEFILE_SOURCE -D_FILE_OFFSET_BITS=64 -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


