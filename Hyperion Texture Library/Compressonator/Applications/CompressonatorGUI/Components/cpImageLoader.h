//=====================================================================
// Copyright 2016 (c), Advanced Micro Devices, Inc. All rights reserved
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files(the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and / or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions :
// 
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//.
//=====================================================================


#ifndef _IMAGELOADER_H
#define _IMAGELOADER_H

#include <QtCore/qstring.h>
#include <QtGui/qimage.h>
#include <QtCore/qlist.h>
#include <QtCore/qfileinfo.h>
#include <QtGui/qpainter.h>
#include "stdlib.h"

#include "Compressonator.h"
#include "Common.h"
#include "cmdline.h"
#include "TextureIO.h"
#include "PluginManager.h"
#include "TC_PluginAPI.h"


enum MIPIMAGE_FORMAT_DECOMPRESSED {
    Format_NONE,
    Format_CPU,
    Format_GPU
};


enum MIPIMAGE_FORMAT {
    Format_QImage,
    Format_OpenGL,
    Format_DirectX,
    Format_Vulkan,
    Format_CPU_HPC
};


enum MIPIMAGE_FORMAT_ERRORS {
    Format_NoErrors,
    Format_InvalidFile,
    Format_CompressedImage,
    Format_NotSupported
};

class CMipImages
{
public:
    CMipImages();
    QList<QImage *> QImage_list[CMP_MIPSET_MAX_DEPTHS];         // This is a QImage list mapping of the mipset. Its contains a list of 
                                            // Frames (CubeMaps 0..CMP_MIPSET_MAX_DEPTHS-1) and MipLevel (0..MaxMipLevel) Images
                                            // for 2D type textures its set to [0][0..MaxMipLevels]
                                            //     Cubemap textures [0..CMP_MIPSET_MAX_DEPTHS-1][0..MaxMipLevels]
    MipSet *mipset;
    MipSet *decompressedMipSet; 

    MIPIMAGE_FORMAT m_MipImageFormat;
    MIPIMAGE_FORMAT_ERRORS m_Error;
    MIPIMAGE_FORMAT_DECOMPRESSED m_DecompressedFormat;
    QString errMsg;
    bool MIPS2QtFailed;
    
};


class CImageLoader
{
public:
    CImageLoader();
    CImageLoader(void *plugin);

    ~CImageLoader();
    
    CMipImages *LoadPluginImage(QString filename, CMP_Feedback_Proc pFeedbackProc = NULL);      // Creates Image & MIP data 
    bool    clearMipImages(CMipImages **MipImages);      // Clears (delete) all Image & MIP data
    void    UpdateMIPMapImages(CMipImages *MipImages);  // Maps MIP levels to Images
    MipSet *QImage2MIPS(QImage *qimage, CMP_Feedback_Proc pFeedbackProc = NULL);            // Converts a QImage to MipSet
    MipSet *LoaderDecompressMipSet(CMipImages *MipImages, Config *decompConfig);
    CMIPS *getCMips() { return m_CMips; };
    CMP_CompressOptions m_options; //options for hdr loading(kneelow, kneehigh, defog and exposure)

private:
    void QImageFormatInfo(QImage *image);
    CMP_FORMAT   QFormat2MipFormat(QImage::Format qformat);

    PluginManager *m_pluginManager;

    CMIPS  *m_CMips;
    MipSet *LoadPluginMIPS(QString filename);       // Use AMD PluginManager
   };


extern float half_conv_float(unsigned short in);
extern bool ProgressCallback(float fProgress, CMP_DWORD_PTR pUser1, CMP_DWORD_PTR pUser2);
#endif
