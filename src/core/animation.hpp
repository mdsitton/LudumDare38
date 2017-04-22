#ifndef ANIMATION_HPP
#define ANIMATION_HPP

class Animation{
public:
    
    Animation(int n_maxFrames, int n_imageWidth, int n_imageHeight, int n_updateTimerBases);
    
    int frameNumber;
    int maxFrames;

    int updateTimer;
    
    float getImageX();
    float getImageY();

    void update();

    int imageX, imageY;

    int imageHeight;
    int imageWidth;

    void setWidth(int n_width);
    void setHeight(int n_height);

    int getWidth();
    int getHeight();

private:
    int m_imageStartX;
    int m_imageStartY;

    void m_setCurrentImage();
    
    int m_updateTimerBase;

};

#endif
