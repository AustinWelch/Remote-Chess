################################################################################
# Automatically-generated file. Do not edit!
################################################################################

SHELL = cmd.exe

# Each subdirectory must supply rules for building sources it contributes
src/ChessAbstraction/%.obj: ../src/ChessAbstraction/%.cpp $(GEN_OPTS) | $(GEN_FILES) $(GEN_MISC_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: Arm Compiler'
	"C:/ti/ccs1020/ccs/tools/compiler/ti-cgt-arm_20.2.2.LTS/bin/armcl" -mv7M4 --code_state=16 --float_support=FPv4SPD16 -me -Ooff --opt_for_speed=5 --include_path="C:/ti/ccs1020/ccs/ccs_base/arm/include" --include_path="C:/Users/Austin/OneDrive/Classes/Project Tidbits/Remote-Chess/Board/src/BoardSupportPackage/DriverLib" --include_path="C:/Users/Austin/OneDrive/Classes/Project Tidbits/Remote-Chess/Board/src/BoardSupportPackage/inc" --include_path="C:/Users/Austin/OneDrive/Classes/Project Tidbits/Remote-Chess/Board/src/BoardSupportPackage/src" --include_path="C:/Users/Austin/OneDrive/Classes/Project Tidbits/Remote-Chess/Board/src/CC3100SupportPackage/board" --include_path="C:/Users/Austin/OneDrive/Classes/Project Tidbits/Remote-Chess/Board/src/CC3100SupportPackage/simplelink/include" --include_path="C:/Users/Austin/OneDrive/Classes/Project Tidbits/Remote-Chess/Board/src/CC3100SupportPackage/simplelink/source" --include_path="C:/Users/Austin/OneDrive/Classes/Project Tidbits/Remote-Chess/Board/src/CC3100SupportPackage/SL_Common" --include_path="C:/Users/Austin/OneDrive/Classes/Project Tidbits/Remote-Chess/Board/src/CC3100SupportPackage/spi_cc3100" --include_path="C:/Users/Austin/OneDrive/Classes/Project Tidbits/Remote-Chess/Board/src/CC3100SupportPackage/wifi_usage" --include_path="C:/Users/Austin/OneDrive/Classes/Project Tidbits/Remote-Chess/Board/src/ChessAbstraction" --include_path="C:/Users/Austin/OneDrive/Classes/Project Tidbits/Remote-Chess/Board/src/HardwareLayer" --include_path="C:/Users/Austin/OneDrive/Classes/Project Tidbits/Remote-Chess/Board/src/G8RTOS" --include_path="C:/ti/ccs1020/ccs/ccs_base/arm/include/CMSIS" --include_path="C:/Users/Austin/OneDrive/Classes/Project Tidbits/Remote-Chess/Board" --include_path="C:/ti/ccs1020/ccs/tools/compiler/ti-cgt-arm_20.2.2.LTS/include" --advice:power=all --define=__MSP432P401R__ --define=ccs -g --c11 --gcc --diag_error=169 --diag_warning=225 --diag_wrap=off --display_error_number --abi=eabi --preproc_with_compile --preproc_dependency="src/ChessAbstraction/$(basename $(<F)).d_raw" --obj_directory="src/ChessAbstraction" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: "$<"'
	@echo ' '


