//
// Created by cwb on 2022/9/8.
//

#ifndef LEARNMETALVULKAN_DEVICEGL_H
#define LEARNMETALVULKAN_DEVICEGL_H

#include "device.h"
namespace backend
{
class HandleGL : public Device
{
public:
    explicit HandleGL(const Info& info);
    ~HandleGL() override;
    void init() override;
};
} // namespace backend

#endif // LEARNMETALVULKAN_DEVICEGL_H
