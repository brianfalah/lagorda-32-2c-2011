################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Comunes/NIPC.c \
../Comunes/manejoArchivo.c \
../Comunes/sockets.c 

OBJS += \
./Comunes/NIPC.o \
./Comunes/manejoArchivo.o \
./Comunes/sockets.o 

C_DEPS += \
./Comunes/NIPC.d \
./Comunes/manejoArchivo.d \
./Comunes/sockets.d 


# Each subdirectory must supply rules for building sources it contributes
Comunes/%.o: ../Comunes/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -D__USE_LARGEFILE -D_LARGEFILE_SOURCE -D_FILE_OFFSET_BITS=64 -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


