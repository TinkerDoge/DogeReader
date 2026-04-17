import os
from PIL import Image

def convert_tarot():
    input_dir = "tarot/cards"
    thumb_dir = "tarot/thumbs"
    resized_dir = "tarot/resized"
    
    if not os.path.exists(thumb_dir):
        os.makedirs(thumb_dir)
    if not os.path.exists(resized_dir):
        os.makedirs(resized_dir)
        
    for i in range(78):
        filename = f"{i}.bmp"
        input_path = os.path.join(input_dir, filename)
        
        if not os.path.exists(input_path):
            print(f"Skipping {input_path}, not found.")
            continue
            
        with Image.open(input_path) as img:
            # 1. Generate Thumb (80px wide)
            w, h = img.size
            thumb_w = 80
            thumb_h = int(h * (thumb_w / w))
            
            thumb_img = img.resize((thumb_w, thumb_h), Image.LANCZOS).convert("L")
            thumb_path = os.path.join(thumb_dir, filename)
            thumb_img.save(thumb_path)
            
            # 2. Generate Resized Main Card (320px wide)
            main_w = 320
            main_h = int(h * (main_w / w))
            
            main_img = img.resize((main_w, main_h), Image.LANCZOS).convert("L")
            main_path = os.path.join(resized_dir, filename)
            main_img.save(main_path)
            
            print(f"Processed {filename}: thumb {thumb_w}x{thumb_h}, resized {main_w}x{main_h}")

if __name__ == "__main__":
    convert_tarot()
