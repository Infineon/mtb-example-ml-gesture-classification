#!/usr/bin/env bash
python data_prepare.py --overlap 0.2
python train.py --sparsity_level 50
python file_manage.py
python gesture_name_gen.py
python calibration.py