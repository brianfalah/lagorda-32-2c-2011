################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../cache.c \
../consola.c \
../funciones.c \
../funcionesManejoEntradas.c \
../mapeoArchivos.c \
../pfs.c \
../utils.c 

OBJS += \
./cache.o \
./consola.o \
./funciones.o \
./funcionesManejoEntradas.o \
./mapeoArchivos.o \
./pfs.o \
./utils.o 

C_DEPS += \
./cache.d \
./consola.d \
./funciones.d \
./funcionesManejoEntradas.d \
./mapeoArchivos.d \
./pfs.d \
./utils.d 


# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -D_FILE_OFFSET_BITS=64 -I"/home/pablo/Documents/TPSO2C2011/PFS/Comun" -I"/home/pablo/Documents/TPSO2C2011/PFS/librerias" -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


