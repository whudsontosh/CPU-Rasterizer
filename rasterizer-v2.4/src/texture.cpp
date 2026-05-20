#include "texture.h"


TEX::Texture TEX::loadBMP(const char* path)
{
    std::ifstream file(path, std::ios::binary);
    if (!file)
        throw std::runtime_error("Failed to open texture");

    unsigned char header[54];
    file.read((char*)header, 54);

    if (header[0] != 'B' || header[1] != 'M')
        throw std::runtime_error("Not a BMP file");

    int dataOffset = *(int*)&header[10];
    int width      = *(int*)&header[18];
    int height     = *(int*)&header[22];
    short bpp      = *(short*)&header[28];

    if (bpp != 8)
        throw std::runtime_error("This loader only handles 8-bit BMP");

    bool flipY = true;
    if (height < 0)
    {
        height = -height;
        flipY = false;
    }

    // --- READ PALETTE (256 colors, 4 bytes each: B G R 0) ---
    const int paletteSize = 256;
    std::vector<unsigned char> palette(paletteSize * 4);
    file.read((char*)palette.data(), palette.size());

    int rowPadded = (width + 3) & (~3);

    file.seekg(dataOffset, std::ios::beg);

    std::vector<unsigned char> indices(rowPadded * height);
    file.read((char*)indices.data(), indices.size());

    Texture tex;
    tex.TexWidth  = width;
    tex.TexHeight = height;
    tex.TexData.resize(width * height);

    for (int y = 0; y < height; y++)
    {
        int srcY = flipY ? (height - 1 - y) : y;

        for (int x = 0; x < width; x++)
        {
            unsigned char idx = indices[srcY * rowPadded + x];

            int pi = idx * 4;

            unsigned char b = palette[pi + 0];
            unsigned char g = palette[pi + 1];
            unsigned char r = palette[pi + 2];

            tex.TexData[y * width + x] = {
                r / 255.0f,
                g / 255.0f,
                b / 255.0f
            };
        }
    }

    return tex;
}
