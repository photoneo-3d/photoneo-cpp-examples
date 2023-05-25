#pragma once

#include <PhoXi.h>

#include <iostream>

namespace utils
{

template<class ElementType>
void printMat2D(const pho::api::Mat2D<ElementType>& map, const std::string& name) {
    if (!map.Empty())
    {
        std::cout << "    "<< name <<":      ("
            << map.Size.Width << " x "
            << map.Size.Height << ") Type: "
            << map.GetElementName()
            << std::endl;
    }
}

} // namespace info
