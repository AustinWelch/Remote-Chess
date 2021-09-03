################################################################################
# Automatically-generated file. Do not edit!
################################################################################

SHELL = cmd.exe

CG_TOOL_ROOT := C:/ti/Code Composer Studio/ccs/tools/compiler/ti-cgt-arm_20.2.2.LTS

GEN_OPTS__FLAG := 
GEN_CMDS__FLAG := 

ORDERED_OBJS += \
"./startup_msp432p401r_ccs.obj" \
"./system_msp432p401r.obj" \
"./BoardSupportPackage/ferris/LCD.obj" \
"./BoardSupportPackage/ferris/RGBLED.obj" \
"./BoardSupportPackage/ferris/uart.obj" \
"./BoardSupportPackage/src/AsciiLib.obj" \
"./BoardSupportPackage/src/BSP.obj" \
"./BoardSupportPackage/src/BackChannelUart.obj" \
"./BoardSupportPackage/src/ClockSys.obj" \
"./BoardSupportPackage/src/Joystick.obj" \
"./BoardSupportPackage/src/bme280.obj" \
"./BoardSupportPackage/src/bme280_support.obj" \
"./BoardSupportPackage/src/bmi160.obj" \
"./BoardSupportPackage/src/bmi160_support.obj" \
"./BoardSupportPackage/src/demo_sysctl.obj" \
"./BoardSupportPackage/src/i2c_driver.obj" \
"./BoardSupportPackage/src/opt3001.obj" \
"./BoardSupportPackage/src/tmp007.obj" \
"./src/Board.obj" \
"./src/LedDriver.obj" \
"./src/main.obj" \
"./src/G8RTOS/G8RTOS_CriticalSection.obj" \
"./src/G8RTOS/G8RTOS_Scheduler.obj" \
"./src/G8RTOS/G8RTOS_SchedulerASM.obj" \
"./src/G8RTOS/G8RTOS_Semaphores.obj" \
"./src/G8RTOS/G8RTOS_Structures.obj" \
"./src/G8RTOS/Queue.obj" \
"./src/HardwareLayer/LedMatrix.obj" \
"../msp432p401r.cmd" \
"C:/Users/myaka/OneDrive - University of Florida/EEL4745 - Microp 2/lib/BoardSupportPackage/DriverLib/msp432p4xx_driverlib.lib" \
$(GEN_CMDS__FLAG) \
-l"C:/Users/myaka/OneDrive - University of Florida/EEL4745 - Microp 2/lib/BoardSupportPackage/DriverLib/msp432p4xx_driverlib.lib" \
-llibc.a \

-include ../makefile.init

RM := DEL /F
RMDIR := RMDIR /S/Q

# All of the sources participating in the build are defined here
-include sources.mk
-include subdir_vars.mk
-include BoardSupportPackage/DriverLib/subdir_vars.mk
-include BoardSupportPackage/ferris/subdir_vars.mk
-include BoardSupportPackage/src/subdir_vars.mk
-include src/subdir_vars.mk
-include src/G8RTOS/subdir_vars.mk
-include src/HardwareLayer/subdir_vars.mk
-include subdir_rules.mk
-include BoardSupportPackage/DriverLib/subdir_rules.mk
-include BoardSupportPackage/ferris/subdir_rules.mk
-include BoardSupportPackage/src/subdir_rules.mk
-include src/subdir_rules.mk
-include src/G8RTOS/subdir_rules.mk
-include src/HardwareLayer/subdir_rules.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(C55_DEPS)),)
-include $(C55_DEPS)
endif
ifneq ($(strip $(C_UPPER_DEPS)),)
-include $(C_UPPER_DEPS)
endif
ifneq ($(strip $(S67_DEPS)),)
-include $(S67_DEPS)
endif
ifneq ($(strip $(S62_DEPS)),)
-include $(S62_DEPS)
endif
ifneq ($(strip $(S_DEPS)),)
-include $(S_DEPS)
endif
ifneq ($(strip $(OPT_DEPS)),)
-include $(OPT_DEPS)
endif
ifneq ($(strip $(C??_DEPS)),)
-include $(C??_DEPS)
endif
ifneq ($(strip $(ASM_UPPER_DEPS)),)
-include $(ASM_UPPER_DEPS)
endif
ifneq ($(strip $(S??_DEPS)),)
-include $(S??_DEPS)
endif
ifneq ($(strip $(C64_DEPS)),)
-include $(C64_DEPS)
endif
ifneq ($(strip $(CXX_DEPS)),)
-include $(CXX_DEPS)
endif
ifneq ($(strip $(S64_DEPS)),)
-include $(S64_DEPS)
endif
ifneq ($(strip $(INO_DEPS)),)
-include $(INO_DEPS)
endif
ifneq ($(strip $(CLA_DEPS)),)
-include $(CLA_DEPS)
endif
ifneq ($(strip $(S55_DEPS)),)
-include $(S55_DEPS)
endif
ifneq ($(strip $(SV7A_DEPS)),)
-include $(SV7A_DEPS)
endif
ifneq ($(strip $(C62_DEPS)),)
-include $(C62_DEPS)
endif
ifneq ($(strip $(C67_DEPS)),)
-include $(C67_DEPS)
endif
ifneq ($(strip $(PDE_DEPS)),)
-include $(PDE_DEPS)
endif
ifneq ($(strip $(K_DEPS)),)
-include $(K_DEPS)
endif
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
ifneq ($(strip $(CC_DEPS)),)
-include $(CC_DEPS)
endif
ifneq ($(strip $(C++_DEPS)),)
-include $(C++_DEPS)
endif
ifneq ($(strip $(C43_DEPS)),)
-include $(C43_DEPS)
endif
ifneq ($(strip $(S43_DEPS)),)
-include $(S43_DEPS)
endif
ifneq ($(strip $(ASM_DEPS)),)
-include $(ASM_DEPS)
endif
ifneq ($(strip $(S_UPPER_DEPS)),)
-include $(S_UPPER_DEPS)
endif
ifneq ($(strip $(CPP_DEPS)),)
-include $(CPP_DEPS)
endif
ifneq ($(strip $(SA_DEPS)),)
-include $(SA_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 
EXE_OUTPUTS += \
ChessBoard.out \

EXE_OUTPUTS__QUOTED += \
"ChessBoard.out" \

BIN_OUTPUTS += \
ChessBoard.hex \

BIN_OUTPUTS__QUOTED += \
"ChessBoard.hex" \


# All Target
all: $(OBJS) $(CMD_SRCS) $(LIB_SRCS) $(GEN_CMDS)
	@$(MAKE) --no-print-directory -Onone "ChessBoard.out"

# Tool invocations
ChessBoard.out: $(OBJS) $(CMD_SRCS) $(LIB_SRCS) $(GEN_CMDS)
	@echo 'Building target: "$@"'
	@echo 'Invoking: Arm Linker'
	"C:/ti/Code Composer Studio/ccs/tools/compiler/ti-cgt-arm_20.2.2.LTS/bin/armcl" -mv7M4 --code_state=16 --float_support=FPv4SPD16 -me -Ooff --opt_for_speed=5 --advice:power=all --define=__MSP432P401R__ --define=ccs -g --c11 --gcc --diag_error=169 --diag_warning=225 --diag_wrap=off --display_error_number --abi=eabi -z -m"ChessBoard.map" --heap_size=1024 --stack_size=512 -i"C:/ti/Code Composer Studio/ccs/ccs_base/arm/include" -i"C:/ti/Code Composer Studio/ccs/tools/compiler/ti-cgt-arm_20.2.2.LTS/lib" -i"C:/ti/Code Composer Studio/ccs/tools/compiler/ti-cgt-arm_20.2.2.LTS/include" --reread_libs --diag_wrap=off --display_error_number --warn_sections --xml_link_info="ChessBoard_linkInfo.xml" --rom_model -o "ChessBoard.out" $(ORDERED_OBJS)
	@echo 'Finished building target: "$@"'
	@echo ' '

ChessBoard.hex: $(EXE_OUTPUTS)
	@echo 'Building files: $(strip $(EXE_OUTPUTS__QUOTED))'
	@echo 'Invoking: Arm Hex Utility'
	"C:/ti/Code Composer Studio/ccs/tools/compiler/ti-cgt-arm_20.2.2.LTS/bin/armhex" --memwidth=8 --romwidth=8 --diag_wrap=off -o "ChessBoard.hex" $(EXE_OUTPUTS__QUOTED)
	@echo 'Finished building: $(strip $(EXE_OUTPUTS__QUOTED))'
	@echo ' '

# Other Targets
clean:
	-$(RM) $(BIN_OUTPUTS__QUOTED)$(EXE_OUTPUTS__QUOTED)
	-$(RM) "startup_msp432p401r_ccs.obj" "system_msp432p401r.obj" "BoardSupportPackage\ferris\LCD.obj" "BoardSupportPackage\ferris\RGBLED.obj" "BoardSupportPackage\ferris\uart.obj" "BoardSupportPackage\src\AsciiLib.obj" "BoardSupportPackage\src\BSP.obj" "BoardSupportPackage\src\BackChannelUart.obj" "BoardSupportPackage\src\ClockSys.obj" "BoardSupportPackage\src\Joystick.obj" "BoardSupportPackage\src\bme280.obj" "BoardSupportPackage\src\bme280_support.obj" "BoardSupportPackage\src\bmi160.obj" "BoardSupportPackage\src\bmi160_support.obj" "BoardSupportPackage\src\demo_sysctl.obj" "BoardSupportPackage\src\i2c_driver.obj" "BoardSupportPackage\src\opt3001.obj" "BoardSupportPackage\src\tmp007.obj" "src\Board.obj" "src\LedDriver.obj" "src\main.obj" "src\G8RTOS\G8RTOS_CriticalSection.obj" "src\G8RTOS\G8RTOS_Scheduler.obj" "src\G8RTOS\G8RTOS_SchedulerASM.obj" "src\G8RTOS\G8RTOS_Semaphores.obj" "src\G8RTOS\G8RTOS_Structures.obj" "src\G8RTOS\Queue.obj" "src\HardwareLayer\LedMatrix.obj" 
	-$(RM) "startup_msp432p401r_ccs.d" "system_msp432p401r.d" "BoardSupportPackage\ferris\LCD.d" "BoardSupportPackage\ferris\RGBLED.d" "BoardSupportPackage\ferris\uart.d" "BoardSupportPackage\src\AsciiLib.d" "BoardSupportPackage\src\BSP.d" "BoardSupportPackage\src\BackChannelUart.d" "BoardSupportPackage\src\ClockSys.d" "BoardSupportPackage\src\Joystick.d" "BoardSupportPackage\src\bme280.d" "BoardSupportPackage\src\bme280_support.d" "BoardSupportPackage\src\bmi160.d" "BoardSupportPackage\src\bmi160_support.d" "BoardSupportPackage\src\demo_sysctl.d" "BoardSupportPackage\src\i2c_driver.d" "BoardSupportPackage\src\opt3001.d" "BoardSupportPackage\src\tmp007.d" "src\G8RTOS\G8RTOS_Scheduler.d" "src\G8RTOS\G8RTOS_Semaphores.d" "src\G8RTOS\G8RTOS_Structures.d" "src\G8RTOS\Queue.d" 
	-$(RM) "src\Board.d" "src\main.d" "src\HardwareLayer\LedMatrix.d" 
	-$(RM) "src\LedDriver.d" "src\G8RTOS\G8RTOS_CriticalSection.d" "src\G8RTOS\G8RTOS_SchedulerASM.d" 
	-@echo 'Finished clean'
	-@echo ' '

.PHONY: all clean dependents
.SECONDARY:

-include ../makefile.targets

