################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Comunes/librerias/array.c \
../Comunes/librerias/commands.c \
../Comunes/librerias/config_loader.c \
../Comunes/librerias/fileio.c \
../Comunes/librerias/log.c \
../Comunes/librerias/scanner.c \
../Comunes/librerias/serializer.c \
../Comunes/librerias/utils.c 

OBJS += \
./Comunes/librerias/array.o \
./Comunes/librerias/commands.o \
./Comunes/librerias/config_loader.o \
./Comunes/librerias/fileio.o \
./Comunes/librerias/log.o \
./Comunes/librerias/scanner.o \
./Comunes/librerias/serializer.o \
./Comunes/librerias/utils.o 

C_DEPS += \
./Comunes/librerias/array.d \
./Comunes/librerias/commands.d \
./Comunes/librerias/config_loader.d \
./Comunes/librerias/fileio.d \
./Comunes/librerias/log.d \
./Comunes/librerias/scanner.d \
./Comunes/librerias/serializer.d \
./Comunes/librerias/utils.d 


# Each subdirectory must supply rules for building sources it contributes
Comunes/librerias/%.o: ../Comunes/librerias/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -D__USE_LARGEFILE -D_LARGEFILE_SOURCE -D_FILE_OFFSET_BITS=64 -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


