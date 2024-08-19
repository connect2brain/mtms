import torch

def test_cuda():
    if torch.cuda.is_available():
        print("CUDA is available!")
        print(f"Device count: {torch.cuda.device_count()}")
        print(f"Current device: {torch.cuda.current_device()}")
        print(f"Device name: {torch.cuda.get_device_name(torch.cuda.current_device())}")
        return True
    else:
        print("CUDA is not available.")
        return False

if __name__ == "__main__":
    success = test_cuda()
    exit(0 if success else 1)
