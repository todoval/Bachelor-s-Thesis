# Bachelor-s-Thesis

OCR for tabular data, inluding preprocessing of the input image.

The software has been tested on Windows 10 (MSCV 2017) and Ubuntu 18 (G++ 7.4). As we do not provide any binaries, the user must compile the project first.

To compile the project, execute the following steps:

Windows:
    1) Clone GitHub directory.
    2) Download and setup cppan.
    3) Run cppan in any directory.
    4) Build the project using CMake (make sure you have installed MSVC~2017 and CMake~3.8).
    5) Set the environment variable TESSDATA_PREFIX to tabularOCR/tessdata directory.

Linux:
    1) Clone GitHub directory.
    2) Install the following libraries: libleptonica-dev, libtesseract-dev, libopencv-dev.
    3) Build the project using CMake (make sure you have installed G++ 7.4 and CMake 3.8).
    4) item Set the environment variable TESSDATA_PREFIX to tabularOCR/tessdata directory.

Follow these steps to run a sample demo:
    1) Take one of the pictures from our sample images directory at \url{https://drive.google.com/open?id=1cPPQc0H2AYUB7jHM8_6KwlQ05YNOrJdY}, for example \texttt{11-1.jpg}, and copy it to \texttt{your-build-directory/bin/Debug}.
    2) From this directory, run command:
            Windows: \texttt{tabularOCR.exe 11-1.jpg}
            Linux: \texttt{./tabularOCR 11-1.jpg}
        In the rest of this user guide, we will be using a unified command call tabularOCR.
    3) The results can be found in a your-build-directory/bin/Debug/results directory as both 11-1.png and 11-1.json.

Note: The input image does not necessarily have to be in the same directory as the tabularOCR compiled binary. However, in such case, the path to the image must be provided.

It is possible to run the program with several options. The explained usage with complete list of options is as follows:

Usage: tabularOCR [-options] (filenames | directory name)
where options include:
    (-e | --enhance) (SIMPLE | GAMMA | EQUALIZATION)
        enhance the contrast of the image before processing
    (-g | --greyscale) (AVG | MIN | MAX | LUMA)
        set the image mode to greyscale before processing
    (-b | --binarize) (OTSU | SAUVOLA)
        binarize image before processing
    -p | --preprocess
        preprocess image with the default preprocessing options before processing
    -sk | --deskew
        deskew image before processing
    --output-json
        output the result in a json file
    --output-image
        output the result in an png file as a bounding box around each cell
		
Examples of usage: 
    tabularOCR -e EQUALIZATION --binarize OTSU 11-1.jpg
    
    This command will perform the table extraction on a binarized 11-1.jpg image (with the exact method of binarization being Otsu) with enhanced contrast (with the exact method of contrast enhancement being histogram equalization).
    
    tabularOCR ./image_directory
    
    This command will process all the images from your-build-directory/bin/Debug/image_directory and save their results into your-build-directory/bin/Debug/results/image_directory directory.
    
    tabularOCR --output-image 11-1.jpg 7-1.jpg 5-1.jpg
    
    This command will process the images 11-1.jpg, 7-1.jpg, 5-1.jpg in the given order and save their results as 11-1.png, 7-1.png and 5-1.png files, excluding JSON output.
    
\end{itemize}