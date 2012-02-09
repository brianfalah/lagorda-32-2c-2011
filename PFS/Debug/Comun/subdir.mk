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
	gcc -D_FILE_OFFSET_BITS=64 -I"/home/pablo/Documents/TPSO2C2011/PFS/Comun" -I"/home/pablo/Documents/TPSO2C2011/PFS/librerias" -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


