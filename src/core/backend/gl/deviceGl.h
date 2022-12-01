//
// Created by cwb on 2022/9/8.
//

#ifndef LEARNMETALVULKAN_DEVICEGL_H
#define LEARNMETALVULKAN_DEVICEGL_H

#include "device.h"
namespace backend
{
class DeviceGl : public Device
{
public:
    explicit DeviceGl(const Info& info);
    ~DeviceGl() override;
    void init() override;
};
} // namespace backend

#endif // LEARNMETALVULKAN_DEVICEGL_H
