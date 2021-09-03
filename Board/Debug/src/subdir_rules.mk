################################################################################
# Automatically-generated file. Do not edit!
################################################################################

SHELL = cmd.exe

# Each subdirectory must supply rules for building sources it contributes
src/%.obj: ../src/%.cpp $(GEN_OPTS) | $(GEN_FILES) $(GEN_MISC_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: Arm Compiler'
	"C:/ti/Code Composer Studio/ccs/tools/compiler/ti-cgt-arm_20.2.2.LTS/bin/armcl" -mv7M4 --code_state=16 --float_support=FPv4SPD16 -me -Ooff --opt_for_speed=5 --include_path="C:/ti/Code Composer Studio/ccs/ccs_base/arm/include" --include_path="C:/Users/myaka/OneDrive - University of Florida/EEL4745 - Microp 2/CCS/ChessBoard/src/ChessAbstraction" --include_path="C:/Users/myaka/OneDrive - University of Florida/EEL4745 - Microp 2/CCS/ChessBoard/src/HardwareLayer" --include_path="C:/Users/myaka/OneDrive - University of Florida/EEL4745 - Microp 2/CCS/ChessBoard/src/G8RTOS" --include_path="C:/Users/myaka/OneDrive - University of Florida/EEL4745 - Microp 2/lib/BoardSupportPackage" --include_path="C:/Users/myaka/OneDrive - University of Florida/EEL4745 - Microp 2/lib/BoardSupportPackage/inc" --include_path="C:/Users/myaka/OneDrive - University of Florida/EEL4745 - Microp 2/lib/BoardSupportPackage/DriverLib" --include_path="C:/ti/Code Composer Studio/ccs/ccs_base/arm/include/CMSIS" --include_path="C:/Users/myaka/OneDrive - University of Florida/EEL4745 - Microp 2/CCS/ChessBoard" --include_path="C:/ti/Code Composer Studio/ccs/tools/compiler/ti-cgt-arm_20.2.2.LTS/include" --advice:power=all --define=__MSP432P401R__ --define=ccs -g --c11 --gcc --diag_error=169 --diag_warning=225 --diag_wrap=off --display_error_number --abi=eabi --preproc_with_compile --preproc_dependency="src/$(basename $(<F)).d_raw" --obj_directory="src" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: "$<"'
	@echo ' '

src/LedDriver.obj: ../src/LedDriver.s $(GEN_OPTS) | $(GEN_FILES) $(GEN_MISC_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: Arm Compiler'
	"C:/ti/Code Composer Studio/ccs/tools/compiler/ti-cgt-arm_20.2.2.LTS/bin/armcl" -mv7M4 --code_state=16 --float_support=FPv4SPD16 -me --opt_for_speed=0 --include_path="C:/ti/Code Composer Studio/ccs/ccs_base/arm/include" --include_path="C:/Users/myaka/OneDrive - University of Florida/EEL4745 - Microp 2/CCS/ChessBoard/src/ChessAbstraction" --include_path="C:/Users/myaka/OneDrive - University of Florida/EEL4745 - Microp 2/CCS/ChessBoard/src/HardwareLayer" --include_path="C:/Users/myaka/OneDrive - University of Florida/EEL4745 - Microp 2/CCS/ChessBoard/src/G8RTOS" --include_path="C:/Users/myaka/OneDrive - University of Florida/EEL4745 - Microp 2/lib/BoardSupportPackage" --include_path="C:/Users/myaka/OneDrive - University of Florida/EEL4745 - Microp 2/lib/BoardSupportPackage/inc" --include_path="C:/Users/myaka/OneDrive - University of Florida/EEL4745 - Microp 2/lib/BoardSupportPackage/DriverLib" --include_path="C:/ti/Code Composer Studio/ccs/ccs_base/arm/include/CMSIS" --include_path="C:/Users/myaka/OneDrive - University of Florida/EEL4745 - Microp 2/CCS/ChessBoard" --include_path="C:/ti/Code Composer Studio/ccs/tools/compiler/ti-cgt-arm_20.2.2.LTS/include" --advice:power=all --define=__MSP432P401R__ --define=ccs -g --c11 --gcc --diag_error=169 --diag_warning=225 --diag_wrap=off --display_error_number --abi=eabi --preproc_with_compile --preproc_dependency="src/$(basename $(<F)).d_raw" --obj_directory="src" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: "$<"'
	@echo ' '


