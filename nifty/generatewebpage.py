import os, os.path
import sys
from jinja2 import Environment, FileSystemLoader

def pass_to_template(paths_to_images, freq_of_images, output_file):
  env = Environment(loader=FileSystemLoader('templates'))
  template = env.get_template('slideshow.html')
  #print paths_to_images

  f = open(output_file, "w")
  f.write(template.render(image_locs=paths_to_images, image_freqs=freq_of_images))
  f.close()


def main(argv):
  if len(argv) != 3:
    print "Usage: python generatewebpage.py input-directoryname output-filename.html"
    exit()
  # Get all the image files
  dir_name = argv[1]
  image_files = os.listdir(dir_name)
  image_files_tosort = [];
  for image_file in image_files:
    q_index = image_file.find("Quote")
    freq = int(image_file[:q_index])
    image_files_tosort.append((image_file, freq))
  image_files_tosort.sort(key=lambda x: x[1])
  image_files_tosort.reverse()
  image_files = [x for (x, y) in image_files_tosort]
  image_freqs = [y for (x, y) in image_files_tosort]
  if dir_name[len(dir_name)-1] == '/':
    dir_name = dir_name[:len(dir_name)-1]
  image_files = [dir_name + '/' + x for x in image_files]
  pass_to_template(image_files, image_freqs, argv[2])
  

if __name__ == '__main__':
  main(sys.argv)
