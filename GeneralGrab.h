#pragma once

#include "ItkCamera.h"

/* @brief：创建数据流和缓冲区。
 *
 * @brief：Create data stream and buffer. */
bool CreateStreamAndBuffer(pItkCamera pCam);

/* @brief：配置数据流。
 *
 * @brief：Configure data stream. */
bool ConfigureStream(pItkCamera pCam);
