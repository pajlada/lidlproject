#define cimg_display 0
#include "CImg.h"
#include "data.hh"
using namespace cimg_library;

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

#include <csignal>
#include <cstdio>
#include <iostream>
#include <string>

static CImg<unsigned char> src, img, img2;
static float alpha = 0.f;
static float imageTime = 0.f;
static unsigned imageWidth = 400;
static unsigned imageHeight = 300;
FILE *fh;
std::unique_ptr<boost::asio::deadline_timer> intervalTimer;
static const boost::posix_time::milliseconds interval(8);

static bool running = true;

const unsigned long imageWH = imageWidth * imageHeight;
static std::size_t bufferSize = imageWH * 3;
static unsigned char *const buffer = new unsigned char[bufferSize];

void InitImage()
{
    std::cout << "Opening pipe...\n";
    fh = fopen("MYPIPE", "w");
    std::cout << "Opened pipe!\n";

    src = CImg<unsigned char>(data_milla, 211, 242, 1, 3, false).resize(400, 300, 1, 3, 1);
    img = src;
    img2 = img;
}

void RenderImage(const boost::system::error_code &)
{
    std::cout << "RenderImage start!\n";
    static unsigned index = 0;
    if (running) {
        intervalTimer->expires_at(intervalTimer->expires_at() + interval);
        intervalTimer->async_wait(boost::bind(RenderImage, boost::asio::placeholders::error));
    }
    std::cout << "Every 16 ms FeelsGoodMan " << index++ << "\n";

    float angle = 0, zoom0 = -0.9f, w2 = 0.5f * img.width(), h2 = 0.5f * img.height();

    unsigned imageIndex = 0;
    cimg_forYC(src, y, k)
    {
        const int xc =
            4 * src.width() + (int)(60 * std::sin((float)y * 3 / src.height() + 10 * imageTime));
        cimg_forX(src, x)
        {
            const float val =
                (float)(src((xc + x) % src.width(), y, 0, k) *
                        (1.3f + 0.20 * std::sin(alpha + k * k * ((float)src.width() / 2 - x) *
                                                            ((float)src.height() / 2 - y) *
                                                            std::cos(imageTime) / 300.0)));
            img(x, y, 0, k) = (unsigned char)(val > 255.0f ? 255 : val);
        }
    }
    const float zoom = 1.0f + (float)(zoom0 + 0.3f * (1 + std::cos(3 * imageTime))),
                rad = (float)(angle * cimg::PI / 180), ca = (float)std::cos(rad) / zoom,
                sa = (float)std::sin(rad) / zoom;
    cimg_forXY(img, x, y)
    {
        const float cX = x - w2, cY = y - h2, fX = w2 + cX * ca - cY * sa,
                    fY = h2 + cX * sa + cY * ca;
        const int X = cimg::mod((int)fX, img.width()), Y = cimg::mod((int)fY, img.height());
        cimg_forC(img, c) img2(x, y, c) = img(X, Y, c);
    }
    // std::string filename("img/" + std::to_string(imageIndex) + "-image.raw");
    // std::string filename("MYPIPE");
    // std::string filename("hehe.rgb");
    // std::cout << "Saving to " << filename << "\n";
    // img2.swap(img).save_rgb(filename.c_str());
    // img2.swap(img).save_rgb_to_buffer(buffer);
    const unsigned char color[] = {16, 32, 64};
    img2.swap(img)
        .draw_text(10 + (100 * (0.5f + (sin(imageTime) / 2.f))), 100, "LOOOOOOOOL 4HEad", color, 1,
                   1.f, 30)
        .save_rgb_to_buffer(buffer);
    fwrite(buffer, sizeof(unsigned char), bufferSize, fh);
    fflush(fh);
    alpha += 0.7f;
    imageTime += 0.05f;
    angle += 0.8f;

    ++imageIndex;
    std::cout << "Render image stop!\n";
}

void InitializeSignalHandlers()
{
    signal(SIGINT, [](int s) {
        running = false;
        std::cout << "QUIT HEHE" << std::endl;
    });
}

void InitializeTimer(boost::asio::io_service &ioService)
{
    intervalTimer.reset(new boost::asio::deadline_timer(ioService, interval));
    intervalTimer->async_wait(boost::bind(RenderImage, boost::asio::placeholders::error));
}

int main(int argc, char **argv)
{
    boost::asio::io_service io;

    InitializeSignalHandlers();

    InitializeTimer(io);

    InitImage();

    io.run();

    fflush(fh);
    fclose(fh);

    pause();

    return 0;
}
