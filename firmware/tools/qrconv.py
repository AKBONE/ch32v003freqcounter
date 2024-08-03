from PIL import Image
import sys

def bmp_to_c_header(bmp_file, header_file):
    # BMPファイルを開く
    with Image.open(bmp_file) as img:
        # 画像が64x64であることを確認
        if img.size != (64, 64):
            raise ValueError("Image size is not 64x64")

        # 画像をグレースケールに変換
        img = img.convert('1')  # 1-bit pixels, black and white

        # ピクセルデータを取得
        pixels = list(img.getdata())

        # Cヘッダーファイルを作成
        with open(header_file, 'w') as header:
            header.write('#ifndef QR_H\n')
            header.write('#define QR_H\n\n')
            header.write('#define QR_WIDTH 64\n')
            header.write('#define QR_HEIGHT 64\n\n')
            header.write('const unsigned char qr_data[] = {\n')

            byte_count = 0
            for y in range(64):
                for x in range(0, 64, 8):
                    byte = 0
                    for bit in range(8):
                        if x + bit < 64:
                            pixel_index = y * 64 + (x + bit)
                            pixel_value = pixels[pixel_index]
                            bit_value = 0 if pixel_value else 1
                            byte = (byte << 1) | bit_value
                        else:
                            byte = (byte << 1)
                    
                    if byte_count % 16 == 0:
                        header.write('\n  ')
                    header.write(f'0x{byte:02X}, ')
                    byte_count += 1
            
            header.write('\n};\n\n')
            header.write('#endif // IMAGE_H\n')

if __name__ == '__main__':
    if len(sys.argv) != 3:
        print("Usage: python qrconv.py qr.bmp qr.h")
        sys.exit(1)

    bmp_file = sys.argv[1]
    header_file = sys.argv[2]
    bmp_to_c_header(bmp_file, header_file)
