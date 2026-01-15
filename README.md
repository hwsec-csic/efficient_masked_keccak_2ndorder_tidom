# KECCAK_ASM


## FILES
- **firmware/mcu**: Contains all files related to STM32 target execution.
    - **/hal**: Libraries and drivers required for proper firmware execution. This folder has been modified from the version provided by ChipWhisperer to enable interrupts, timers, and other functionalities.
    - **/keccak_unmasked_Opt_ASSEMBLY**: Unmasked Keccak source code.
        - **Keccak1600NoOpt.s**: ASM implementation of the unmasked Keccak permutation (ARM Cortex-M3).
        - **KeccakFW.c**: High-level framework.
    - **/keccak_masked_TI3shares_1stOrder_Opt_ASSEMBLY/no-remasking**: Masked Keccak source code (1st-order Threshold Implementation with 3 shares).
        - **Keccak1600NoOpt.s**: ASM implementation of the masked Keccak permutation (ARM Cortex-M3).
        - **KeccakFW.c**: High-level framework.
    - **/keccak_masked_TI3shares_2ndOrder_Opt_ASSEMBLY/no-remasking**: Masked Keccak source code (1st+2nd-order Threshold Implementation with 3 shares).
        - **Keccak1600NoOpt.s**: ASM implementation of the masked Keccak permutation (ARM Cortex-M3).
        - **KeccakFW.c**: High-level framework.

- **jupyter/KECCAK_Notebooks**: Contains notebooks for using ChipWhisperer, capturing traces, and performing TVLA.
    - **CW_Capture.ipynb**: ChipWhisperer client for capturing and storing traces.
    - **TVLA.ipynb**: Processes traces captured with `CW_Capture.ipynb` for TVLA analysis.
 

## INSTALLATION
1. Install **ChipWhisperer 6.0.0** following the instructions at [https://chipwhisperer.readthedocs.io/en/latest/installation.html](https://chipwhisperer.readthedocs.io/en/latest/installation.html).
2. Copy the folders **firmware/mcu/** and **jupyter/KECCAK_Notebooks/** into the **chipwhisperer/** directory, replacing all existing files.

## HOW TO USE

### CAPTURING TRACES
1. Open the notebook **jupyter/KECCAK_Notebooks/CW_Capture.ipynb**.
2. Define your parameters in the first cell:
   - **base_dir_traces**: Path where the experiment subfolder storing all traces will be created.
   - **experiment_name**: Name of the subfolder to be created within _base_dir_traces_, where all traces for this experiment will be stored.
   - **masking_list**: List of implementations to execute sequentially, if multiple implementations are desired.
   - **remasking_list**: List of labels associated with _lista_remasking_, indicating whether mask refreshing should be applied in each corresponding implementation.
   - **iteration_start**: ID of the first set of 5000 traces (used for trace organization).
   - **iteration_end**: ID of the last set of 5000 traces (used for trace organization). Together with _iteration_start_, it defines the total number of traces to capture.
   - **iterations_per_full_trace**: Number of captures containing 24,000 samples in the same trace (with the appropriate offset to start the next capture immediately after the previous one). Set to 3 for unmasked implementations and 11 for masked implementations.
3. Execute the first code cell to apply these parameters.
4. Execute the second code cell to initialize the ChipWhisperer platform.
5. Execute the third code cell to compile all source code. This generates the `.hex` file to flash the microcontroller. This step can be skipped if the `.hex` file already contains the correct compilation.
6. Execute the fourth code cell to define functions for capturing traces in `.h5` format. Each generated file contains 5000 traces: 2500 with fixed plaintexts and 2500 with random plaintexts, alternating between them. The number of traces per file can be modified via the _TRACES_PER_FILE_ parameter.
7. Finally, execute the fifth code cell to launch trace capture.

### PERFORMING TVLA
1. Open the notebook **jupyter/KECCAK_Notebooks/TVLA.ipynb**.
2. Define your parameters in the first cell:
   - **base_path**: Path to the experiment subfolder containing all traces (must be _base_dir_traces_ + _experiment_name_, consistent with the paths defined in `CW_Capture.ipynb`).
   - **num_iterations**: Number of captures containing 24,000 samples per trace (must match _iterations_per_full_trace_ as defined in `CW_Capture.ipynb`).
3. Execute the first code cell to apply these parameters.
4. Execute the second code cell to start the TVLA computation.
5. Execute the third, fourth, and fifth code cells to visualize the TVLA results.
