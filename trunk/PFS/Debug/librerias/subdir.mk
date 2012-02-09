################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../librerias/array.c \
../librerias/commands.c \
../librerias/config_loader.c \
../librerias/fileio.c \
../librerias/log.c \
../librerias/scanner.c \
../librerias/serializer.c \
../librerias/utils.c 

OBJS += \
./librerias/array.o \
./librerias/commands.o \
./librerias/config_loader.o \
./librerias/fileio.o \
./librerias/log.o \
./librerias/scanner.o \
./librerias/serializer.o \
./librerias/utils.o 

C_DEPS += \
./librerias/array.d \
./librerias/commands.d \
./librerias/config_loader.d \
./librerias/fileio.d \
./librerias/log.d \
./librerias/scanner.d \
./librerias/serializer.d \
./librerias/utils.d 


# Each subdirectory must supply rules for building sources it contributes
librerias/%.o: ../librerias/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -D_FILE_OFFSET_BITS=64 -I"/home/pablo/Documents/TPSO2C2011/PFS/Comun" -I"/home/pablo/Documents/TPSO2C2011/PFS/librerias" -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


