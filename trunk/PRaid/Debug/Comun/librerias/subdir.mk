################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Comun/librerias/array.c \
../Comun/librerias/commands.c \
../Comun/librerias/config_loader.c \
../Comun/librerias/fileio.c \
../Comun/librerias/log.c \
../Comun/librerias/scanner.c \
../Comun/librerias/serializer.c \
../Comun/librerias/utils.c 

OBJS += \
./Comun/librerias/array.o \
./Comun/librerias/commands.o \
./Comun/librerias/config_loader.o \
./Comun/librerias/fileio.o \
./Comun/librerias/log.o \
./Comun/librerias/scanner.o \
./Comun/librerias/serializer.o \
./Comun/librerias/utils.o 

C_DEPS += \
./Comun/librerias/array.d \
./Comun/librerias/commands.d \
./Comun/librerias/config_loader.d \
./Comun/librerias/fileio.d \
./Comun/librerias/log.d \
./Comun/librerias/scanner.d \
./Comun/librerias/serializer.d \
./Comun/librerias/utils.d 


# Each subdirectory must supply rules for building sources it contributes
Comun/librerias/%.o: ../Comun/librerias/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -I"/home/pablo/workspace/PRaid/Comun" -I/home/pablo/workspace/Comun -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


