# MirrorMirror
Project webpage: http://www.eecs.berkeley.edu/~junyanz/projects/mirrormirror/  
Contact: Jun-Yan Zhu (junyanz at eecs dot berkeley dot edu)


## Overview
This is the authors' implementation of Expression Training App described in:  
Mirror Mirror: Crowdsourcing Better Portraits  
Jun-Yan Zhu, Aseem Agarwala, Alexei A. Efros, Eli Shechtman and Jue Wang  
In ACM Transactions on Graphics (Proceedings of SIGGRAPH Asia 2014)

This App is used for training users to mimic their best expressions. The App takes input from a webcam/video and displays the current expression along with its attractiveness and seriousness scores, computed in real-time.  The user can specify a target expression to mimic. The App then shows three windows; the current expression, the target expression, and an aligned and a blended cross-fade between the two. The cross-fade oscillates between the target and current expression once per two seconds, so that the subject can examine differences between the two expressions. See Section 6 in the original paper for details.

This software includes two modules:  
(1) Learning attractive/serious SVM models given expressions and annotations.  
(2) Expression Training App.  

The code can only be used for non-commercial purposes. Please cite our paper if you use our code and data for your research.


## Third party libraries
* OpenCV:  
  - sudo apt-get install libopencv-dev
* OpenGL and GLUT:  
  - sudo apt-get install libglu1-mesa-dev freeglut3-dev mesa-common-dev  
  - sudo apt-get install libxmu-dev libxi-dev  
* HOG feature code (included)  
* CLM Tracker by Jason Saragih (included)  
* LIBSVM (included)  


## Installation (Linux)
* Download and unzip the code.
* mkdir cbuild
* cd cbuild
* cmake ../
* make


## Quick Examples
An example for learning attractive/serious svm models:  
./MirrorMirror -m 0 ./data/test/ ./data/models/

An example for Expression Training App using trained model and video file:  
./MirrorMirror -m 1 -i ./data/video.avi ./data/target.png ./data/models/

An example for Expression Training App using cross-subject model and webcam:  
./MirrorMirror -m 1 your/target/image/ ./models/svm/


## Command Usage ([ ] : options)
Command line interface: ./MirrorMirror [-v verbose] [-m mode] [-i input_video] [-s save_dir] data_file model_dir  

-v verbose: shows details about the runtime output (default = 1)  
  - 0 -- no output  
  - 1 -- some output  

-m mode: set the training mode (default = 0)  
  - 0 -- learning attractive/serious svm models  
  - 1 -- expression training App  

-i input_video: specify the video input; if no video is specified, the program will use webcam.  

-s save_dir: specify the directory where the results will be saved.  

data_file: set the directory/path of the input data:  
- if m = 0, data_file specifies the directory of images and scores used for svm training;
- if m = 1, data_file is the path of the selected target expression.  

model_dir: set the directory for svm models.
 - Cross-subject svm models trained on 11 subjects are included: ./models/svm


## User Interaction
1. The program displays three windows:  
   - The "Target Expression" window shows the target expression selected by the user.
   - The "Current Expression" window shows the current expression either from video or from webcam.
   - The "MirrorMirror" window shows an aligned and a blended cross-fade between the target. It also shows the attractiveness and seriousness scores of current expression.

2. User Interaction: "MirrorMirror" window takes the user's keyboard input.
   - 's':  start the tracker.  
   - 'r':  restart the tracker if the tracker fails.  
   - 'p':  save the current expression  
   - 'd':  pause the system to see fine-grained differences at a frozen moment of time.
   - 'q':  quite the program


## Training Data Preparation
The training data directory should contain:  
(1) /imgs/: expression images.  
(2) /scores/: attractive/serious scores  
(3) /image.txt: image list. See "./data/test/" for an example.
