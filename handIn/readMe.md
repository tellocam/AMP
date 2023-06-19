# This is the Submission of the following students in the Advanced Multiprocessor Class:
    Camilo Tello Fachin, M#: 12127084
    Manuela Maria Raidl, M#: 01427517
    Iris Grze, M#: 01620781

# Contents of Submission
    - Report in PDF format
    - Folder NebulaData which contains .txt's used for the plots in the report
    - Folder plots which contains all plots in the report
    - Folder framework where the make commands can be used
        - Folder src which contains all the C-files for the locks

# Makefile commands to get a small-plot on Nebula
These make commands should also work locally, if slurm (and the srun command) is available in your environment.
 ```bash
 $cd framework
 $mkdir build
 $make all
 $make small-bench
 $make small-plot
 ```
After succesfully running the last of these make commands, the resulting plot will be stored to the /framework/plots folder and 
the corresponding data in the /framework/data folder.



