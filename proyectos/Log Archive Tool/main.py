import zipfile
import os
import sys

def compress(path):
    try:
        if os.path.isfile(path):
            print("Comprimiendo archivo...")
            with zipfile.ZipFile(path + ".zip", "w", zipfile.ZIP_DEFLATED, allowZip64=True) as zip:
                zip.write(path, arcname=os.path.basename(path))
            os.remove(path)
            print("Comprimido correctamente.")
        elif os.path.isdir(path):
            zip_path = f"{path.rstrip(os.sep)}.zip"
            print(f"Comprimiendo directorio completo en: {zip_path}")
            with zipfile.ZipFile(zip_path, "w", zipfile.ZIP_DEFLATED, allowZip64=True) as zip:
                for root, dirs, files in os.walk(path):
                    for file in files:
                        file_path = os.path.join(root, file)
                        arcname = os.path.relpath(file_path, start=path)
                        zip.write(file_path, arcname=arcname)
            print(f"Directorio comprimido correctamente en {zip_path}")
        else:
            print(f"El path {path} no es un archivo ni un directorio válido.")
    except Exception as e:
        print(f"Error general: {e}")

if __name__ == "__main__":
    if len(sys.argv) == 2:
        compress(sys.argv[1])
    else:
        print("Usage: python main.py <path>")