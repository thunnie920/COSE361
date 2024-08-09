# CycleVAE 실험 코드
## 폴더 test
- corpus: VCC2018 코퍼스
- preprocess: 각 코퍼스 별 preprocess 코드 모음 (현재는 VCC2018만 존재)
- data: VCC2018 코퍼스 preprocess된 결과 저장
- model: 각 방법을 적용한 VAE 모델 저장
- result: 각 VAE 모델로 변환한 음성 저장
- calculate: mcd, msd, gv 계산하는 코드 모음
- stats: 모델별 mcd, msd 결과 저장
- log: 코드 실행시 콘솔에 출력되는 부분 텍스트로 저장
## 파일
- model.py: PyTorch 기반 VAE 모델
- train_base.py: VAE1 VAE2 VAE3 MD 학습
- train_vae3_next.py: VAE3 모델 기반으로 각 방법 적용하여 학습
- train_md_next.py: MD 모델 기반으로 각 방법 적용하여 학습
- convert.py: 학습된 VAE로 음성 변환
- print_stat.py: 계산된 mcd, msd의 통계값 출력
- loss.py: loss 계산 및 log 출력과 관련된 모듈
- speech_tools.py: 기타 필요한 함수
- run.sh: 실험 진행 스크립트

## 세팅방법 (다운받은 프로젝트 폴더 내에서 실행)
0. 가상환경 생성 <code><pre> virtualenv </code></pre> <code><pre> source venv/bin/activate </code></pre>
1. 패키지 설치 <code><pre> pip3 install torch pyworld pysptk h5py numpy kaldi-io librosa scipy scikit-learn fastdtw </code></pre>
2. VCC2018 데이터 다운 https://datashare.is.ed.ac.uk/handle/10283/3061 접속
3. training data for building parallel and non-parallel VC systems released to participants (117.0Mb) 다운로드 후 압축 해제
   <pre><code>	wget https://datashare.ed.ac.uk/bitstream/handle/10283/3061/vcc2018_database_training.zip    </code></pre>
4. evaluation data (source speaker's data) released to participants (31.79Mb) 다운로드 후 압축 해제\n
   <pre><code> wget https://datashare.ed.ac.uk/bitstream/handle/10283/3061/vcc2018_database_evaluation.zip </code></pre>
5. vcc2018_training 폴더를 corpus 폴더에 train이라는 이름으로 저장
6. vcc2018_evaluation 폴더를 corpus 폴더에 test라는 이름으로 저장
7. 데이터 전처리 <code><pre> python3 preprocess/preprocess-vcc2018.py </code></pre>
8. kaldi 설치 - ppg 파일 추출시 필요 (8~11번은 필요 없음)
9. 다시 ppg폴더로 와서 model/timit_sp_ppg_mono 폴더 생성후 아래 구글드라이브 링크의 .zip 파일 다운후 압축해제
10. https://drive.google.com/file/d/1cBmWiQ3GYW9uvrm_AeModCwBT5b-fDVQ/view?usp=sharing 
11. 다시 ppg 폴더로 와서 <code><pre> python3 ppg_vcc2018.py </code></pre> 
12. VAE 학습 -> Voice conversion -> MCD -> CycleVAE 학습 -> Voice conversion -> MCD <code><pre> bash run_all.sh </code></pre>
13. 각 모델별 MCD 및 MSD 값 확인 <code><pre> python3 print_stat.py </code></pre> 
14. Loss graph 출력 <code><pre> bash run_graph.sh </code></pre>

