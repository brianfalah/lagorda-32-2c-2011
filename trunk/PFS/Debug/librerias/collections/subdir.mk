################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../librerias/collections/list.c \
../librerias/collections/queue.c 

OBJS += \
./librerias/collections/list.o \
./librerias/collections/queue.o 

C_DEPS += \
./librerias/collections/list.d \
./librerias/collections/queue.d 


# Each subdirectory must supply rules for building sources it contributes
librerias/collections/%.o: ../librerias/collections/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -D_FILE_OFFSET_BITS=64 -I"/home/pablo/Documents/TPSO2C2011/PFS/Comun" -I"/home/pablo/Documents/TPSO2C2011/PFS/librerias" -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


