#include <iostream>
#include <torch/torch.h>
#include <png++/png.hpp>
#include <utils/vision_utils.hpp>


int main(int argc, char *argv[]){

    auto VU= VisionUtils();
    torch::Device device(torch::kCUDA);
    const std::string modelName = "mosaic_cpp.pt";
    const std::string content_image_path = "amber.png";
    auto module = torch::jit::load(modelName, device);

    //RGB
    png::image<png::rgb_pixel> imageI(content_image_path);
    torch::Tensor tensor = VU.pngToTorch(imageI, device);
    std::cout << "C:" << tensor.size(0) << " H:" << tensor.size(1) << " W:" << tensor.size(2) << std::endl;
    tensor = tensor.
            to(torch::kFloat). // For inference
            unsqueeze(-1). // Add batch
            permute({ 3, 0, 1, 2}). // Fix order, now its {B,C,H,W}
            to(device);

    VU.tensorDIMS(tensor);
    torch::Tensor out_tensor = module.forward({ tensor }).toTensor();

    VU.tensorDIMS(out_tensor); // D=:[1, 3, 320, 480]
    out_tensor = out_tensor.to(torch::kFloat32).detach().cpu().squeeze(); //Remove batch dim, must convert back to torch::kU8
    VU.tensorDIMS(out_tensor); // D=:[3, 320, 480]
    png::image<png::rgb_pixel> imageO = VU.torchToPng(out_tensor);
    imageO.write(content_image_path + "_" + modelName + "-out.png");

    return 0;
}