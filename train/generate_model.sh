#!/usr/bin/env bash
python data_prepare.py --savepkl False --overlap 0.2
python train.py
python file_manage.py
python gesture_name_gen.py
python calibration.py