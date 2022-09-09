//
// Created by cwb on 2022/9/8.
//

#ifndef LEARNMETALVULKAN_DEVICEGL_H
#define LEARNMETALVULKAN_DEVICEGL_H

#include "device.h"
namespace backend
{
class DeviceGL : public Device
{
public:
    explicit DeviceGL(const Info& info);
    ~DeviceGL() override;
    void init() override;
};
} // namespace backend

#endif // LEARNMETALVULKAN_DEVICEGL_H
