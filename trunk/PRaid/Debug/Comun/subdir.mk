################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Comun/NIPC.c \
../Comun/manejoArchivo.c \
../Comun/sockets.c 

OBJS += \
./Comun/NIPC.o \
./Comun/manejoArchivo.o \
./Comun/sockets.o 

C_DEPS += \
./Comun/NIPC.d \
./Comun/manejoArchivo.d \
./Comun/sockets.d 


# Each subdirectory must supply rules for building sources it contributes
Comun/%.o: ../Comun/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -I"/home/pablo/workspace/PRaid/Comun" -I/home/pablo/workspace/Comun -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


