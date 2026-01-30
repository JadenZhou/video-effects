# Video-Special Effects

## **Overview**

This project implements a real-time video processing application using C++ and OpenCV4. It features a variety of filters ranging from basic color manipulations to advanced deep-learning-based depth estimation and face detection.

---

## **Installation & Building**

### **Dependencies**

* **OpenCV 4.x**
* **ONNX Runtime** (Required for Depth Anything V2)

### **Compilation**

A `makefile` is provided in the `src` directory. To build the project, navigate to the `src` folder and run:

```bash
make all

```

This will generate the following executables in the `bin/` directory:

* `img`: A simple utility to read and display an image.
* `vid`: The main live video processing application.
* `img-effects`: A utility for applying filters to static images (Extension).

---

## **Usage & Key Bindings**

While running the `vid` application, use the following keys to toggle effects and interact with the stream:

| Key | Effect / Action |
| --- | --- | 
| **q** | Quit the application | 
| **s** | Save the current frame to `out/` | 
| **g** | Toggle OpenCV Greyscale (`cvtColor`) | 
| **h** | Toggle Custom Greyscale () | 
| **p** | Toggle Sepia Tone | 
| **b** | Toggle  Blur (Separable Filter) | 
| **x** | Toggle X-Sobel Gradient (Vertical Edges) | 
| **y** | Toggle Y-Sobel Gradient (Horizontal Edges) | 
| **m** | Toggle Gradient Magnitude | 
| **l** | Toggle Blur and Quantize | 
| **f** | Toggle Face Detection | 
| **d** | Toggle Depth Estimation (DA2 Network) | 

### **Extra Effects**

* **n**: Negative Image (Pixel-wise)
* **e**: Emboss Effect (Area computation via Sobel)
* **u**: Face Pixelation (Face detection-based)

---

## **Extensions Implemented**

1. **Still Image Processing Utility (`img-effects`):** A separate tool that applies all project filters to static files and saves them with descriptive, timestamped filenames.
2. **Sepia Vignetting:** Enhanced the base sepia filter by adding a radial darkening effect toward the edges of the frame.
3. **Depth Fog:** Uses the Depth Anything V2 output to simulate atmospheric fog, where the intensity of the gray overlay is mapped to pixel distance.
4. **Edge Heatmap:** A custom visualization for the gradient magnitude that maps intensity to a color heatmap (stored as `14-edge-heatmap.png`).

---

## **Project Structure**

* `bin/`: Compiled executables.
* `data/`: Model files (`.onnx`), Haar cascades (`.xml`), and sample images.
* `include/`: Header files (`.h`, `.hpp`).
* `src/`: Source code (`.cpp`) and the `makefile`.
* `out/`: Processed images and performance timing data (`time.txt`).

---

## **Acknowledgements**

* **Depth Anything V2:** Used for monocular depth estimation.
* **OpenCV Documentation:** Consulted for image manipulation and GUI callbacks.
* **Handout Samples:** Utilized skeleton code for video capture and makefile structure.
