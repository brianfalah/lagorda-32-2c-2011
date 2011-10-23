################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../funcionesRaid.c \
../praid.c \
../threadRaid.c 

OBJS += \
./funcionesRaid.o \
./praid.o \
./threadRaid.o 

C_DEPS += \
./funcionesRaid.d \
./praid.d \
./threadRaid.d 


# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -I"/home/pablo/workspace/PRaid/Comun" -I/home/pablo/workspace/Comun -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


