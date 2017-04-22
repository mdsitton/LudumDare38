#include <iostream>

#include "animation.hpp"

using namespace std;

Animation::Animation(int n_maxFrames, int n_imageWidth, int n_imageHeight, int n_updateTimerBase):
maxFrames(n_maxFrames),
imageWidth(n_imageWidth),
imageHeight(n_imageHeight),
m_updateTimerBase(n_updateTimerBase){
    m_imageStartX = 0;
    m_imageStartY = 0;
    imageX = m_imageStartX;
    imageY = m_imageStartY;
    updateTimer = m_updateTimerBase;
    frameNumber = 1;
}

void Animation::m_setCurrentImage(){ // Sets the x and y for the image, not actually setting an image
    imageX = imageWidth * frameNumber;
    imageY = imageHeight * frameNumber;
}

float Animation::getImageX(){
    return (imageX/imageWidth/2.0f + .5f); // Convert to NDC
}

float Animation::getImageY(){
    return (imageY/imageHeight/2.0f + .5f); // Convert to NDC
}

void Animation::setWidth(int n_width){
    imageWidth = n_width;
}

void Animation::setHeight(int n_height){
    imageWidth = n_height;
}

int Animation::getWidth(){
    return imageWidth;
}

int Animation::getHeight(){
    return imageHeight;
}

void Animation::update(){
    if(frameNumber < maxFrames)
	frameNumber++;
    else
	frameNumber = 1;
    m_setCurrentImage();
}
