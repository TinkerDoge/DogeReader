import sys
import os
import fitz  # PyMuPDF
from PIL import Image

def resize_and_fit(img, size=(480, 800), fill_color=(255, 255, 255)):
    """
    Resize, crop, or pad the image to fit exactly size (width, height).
    Maintains aspect ratio, pads with fill_color if needed.
    """
    target_w, target_h = size
    img_ratio = img.width / img.height
    target_ratio = target_w / target_h

    if img_ratio > target_ratio:
        # Image is wider than target: fit width, pad height
        new_w = target_w
        new_h = round(target_w / img_ratio)
    else:
        # Image is taller than target: fit height, pad width
        new_h = target_h
        new_w = round(target_h * img_ratio)
    
    img = img.resize((new_w, new_h), Image.LANCZOS)
    new_img = Image.new("RGB", size, fill_color)
    paste_x = (target_w - new_w) // 2
    paste_y = (target_h - new_h) // 2
    new_img.paste(img, (paste_x, paste_y))
    return new_img

def main():
    if len(sys.argv) != 3:
        print("Usage: python extract_tarot_cards.py <input_pdf> <output_folder>")
        sys.exit(1)
        
    pdf_path = sys.argv[1]
    out_dir = sys.argv[2]
    os.makedirs(out_dir, exist_ok=True)
    
    doc = fitz.open(pdf_path)
    
    # Grid settings for the 6-per-page A4 PDF (3 columns, 2 rows)
    cols = 3
    rows = 2
    card_count = 0
    
    for page_num in range(len(doc)):
        page = doc[page_num]
        # Use high DPI for better quality extraction
        pix = page.get_pixmap(dpi=300)
        page_img = Image.frombytes("RGB", [pix.width, pix.height], pix.samples)
        
        # Calculate cell dimensions
        # Visual analysis shows cards occupy the top ~75% of the A4 page height
        w, h = page_img.size
        content_h = h * 0.72  # Adjusted slightly for more precise cropping
        
        cell_w = w / cols
        cell_h = content_h / rows
        
        for r in range(rows):
            for c in range(cols):
                if card_count >= 78:
                    break
                    
                left = c * cell_w
                top = r * cell_h
                right = left + cell_w
                bottom = top + cell_h
                
                # Crop individual card from the page
                card_img = page_img.crop((left, top, right, bottom))
                
                # Resize and fit to 800x480 for the Eink reader
                card_img = resize_and_fit(card_img)
                
                out_path = os.path.join(out_dir, f"{card_count}.bmp")
                card_img.save(out_path, "BMP")
                print(f"Saved {out_path}")
                card_count += 1
                
    print(f"Done. {card_count} cards extracted from {len(doc)} pages.")

if __name__ == "__main__":
    main()
