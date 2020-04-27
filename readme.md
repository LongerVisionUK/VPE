### How to build, install and use VPI module

* make help           - help info
* make                - make all with VPI library and transcoder-pcie driver
* make vpi            - make only VPI library
* make drivers        - make only transcoder-pcie driver
* make install        - copy the vpe sdk libs to default search path
                          install the pcie driver
                          run "sudo make install" if permission denied
* make clean          - make clean VPI and drivers

### How to make FFmpeg
* How to make FFmpeg
  ```
  cd ffmpeg
  ./configure --enable-vpe  --extra-ldflags="-L/lib/vpe" --extra-libs="-lvpi"
  make
  ```
