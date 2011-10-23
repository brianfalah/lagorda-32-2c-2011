################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Comun/librerias/collections/list.c \
../Comun/librerias/collections/queue.c 

OBJS += \
./Comun/librerias/collections/list.o \
./Comun/librerias/collections/queue.o 

C_DEPS += \
./Comun/librerias/collections/list.d \
./Comun/librerias/collections/queue.d 


# Each subdirectory must supply rules for building sources it contributes
Comun/librerias/collections/%.o: ../Comun/librerias/collections/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -I"/home/pablo/workspace/PRaid/Comun" -I/home/pablo/workspace/Comun -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


