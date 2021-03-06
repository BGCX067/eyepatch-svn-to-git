#define EYEPATCH_VERSION L"0.75"

// Listview Icon size
#define LISTVIEW_SAMPLE_X 64
#define LISTVIEW_SAMPLE_Y 64

// Quicktest Window Size (in images)
#define QUICKTEST_X 3
#define QUICKTEST_Y 3

// Haar Training parameters
#define HAAR_SAMPLE_X 24
#define HAAR_SAMPLE_Y 24
#define MAX_SAMPLES 100
#define MIN_HAAR_STAGES 4
#define START_HAAR_STAGES 10

// Color matching parameters
#define COLOR_MIN_AREA 100
#define COLOR_MAX_AREA 120000
#define COLOR_VMIN 15
#define COLOR_VMAX 230
#define COLOR_SMIN 30

// shape matching parameters
#define SHAPE_MIN_CONTOUR_POINTS 40
#define SHAPE_CANNY_EDGE_FIND 220
#define SHAPE_CANNY_EDGE_LINK 50
#define SHAPE_CANNY_APERTURE 3

// SIFT matching parameters
/* the maximum number of keypoint NN candidates to check during BBF search */
#define KDTREE_BBF_MAX_NN_CHKS 200
/* threshold on squared ratio of distances between NN and 2nd NN */
#define NN_SQ_DIST_RATIO_THR 0.49
#define SIFT_MIN_RANSAC_FEATURES 4

// Motion parameters
/* history image and deltas are in frames, not seconds */
#define MOTION_MHI_DURATION 10.0
#define MOTION_MAX_TIME_DELTA 5.0
#define MOTION_MIN_TIME_DELTA 1.0
/* number of images used to compute silhouette */
#define MOTION_NUM_IMAGES 4
#define MOTION_NUM_HISTORY_FRAMES 10
/* amount of pixel difference considered motion */
#define MOTION_DIFF_THRESHOLD 30

// Gesture parameters
// minimum and maximum number of frames required for a valid gesture
#define GESTURE_MIN_TRAJECTORY_LENGTH 5
#define GESTURE_MAX_TRAJECTORY_LENGTH 150

// optical flow constants
#define FLOW_MAX_TRACK_FEATURES 200
#define FLOW_MAX_ERROR_THRESHOLD 200
// reduce motion from pixel jitter by ignoring motion less than this threshold
#define FLOW_MIN_MOTION_THRESHOLD 1.2
// clear out the trajectory after this many frames with no motion
#define FLOW_INACTIVE_THRESHOLD 20

// OneDollar Recognizer class constants
#define NumTemplates 16
#define NumPoints 64
#define GESTURE_SQUARE_SIZE 250.0
#define GESTURE_BACKREC_STEPSIZE 10
#define HalfDiagonal (0.5 * sqrt(250.0 * 250.0 + 250.0 * 250.0))
#define AngleRange 45.0
#define AnglePrecision 2.0
#define Phi (0.5 * (-1.0 + Math.sqrt(5.0)))

// Tesseract OCR parameters
#define TESSERACT_MAX_CHARS 4096

// minimum number of background subtraction frames to "spin up" background model
#define BACKGROUND_SUBTRACTION_MIN_FRAMES 10
// number of frames to discard at beginning before starting to build background model
#define BACKGROUND_SUBTRACTION_DISCARD_FRAMES 5

// OSC parameters
#define OSC_OUTPUT_BUFFER_SIZE 1024
#define OSC_ADDRESS "127.0.0.1"
#define OSC_PORT 7000

// TCP parameters
#define TCP_LISTEN_PORT 8000
#define TCP_OUTPUT_BUFFER_SIZE 8192

// control placement
#define WINDOW_X 1024
#define WINDOW_Y 768
#define VIDEO_X 640
#define VIDEO_Y 480
#define SLIDER_Y 60
#define EXAMPLEWINDOW_X 0
#define EXAMPLEWINDOW_Y (VIDEO_Y+SLIDER_Y)
#define EXAMPLEWINDOW_WIDTH VIDEO_X
#define EXAMPLEWINDOW_HEIGHT 185
#define FILTERIMAGE_WIDTH 240
#define FILTERIMAGE_HEIGHT 180
#define GUESSMASK_WIDTH 320
#define GUESSMASK_HEIGHT 240
#define FILTERLIBRARY_WIDTH 670

// save recognizer: folder prefixes and file names
#define FILE_FRIENDLY_NAME L"\\name.dat"
#define FILE_DATA_NAME L"\\data.dat"
#define FILE_THRESHOLD_NAME L"\\threshold.dat"
#define FILE_CONTOUR_NAME L"\\data.xml"
#define FILE_CASCADE_NAME L"\\classifier.xml"
#define FILE_DEMOIMAGE_NAME L"\\demo-image.jpg"
#define FILE_SIFTIMAGE_NAME L"\\sift-image.jpg"
#define FILE_CLASSIFIER_PREFIX L"epc"
#define FILE_POSIMAGE_PREFIX L"\\pos"
#define FILE_NEGIMAGE_PREFIX L"\\neg"
#define FILE_MOTIMAGE_PREFIX L"\\mot"
#define FILE_RNGIMAGE_PREFIX L"\\rng"
#define FILE_IMAGE_EXT L".jpg"
#define FILE_MOTIONIMAGE_EXT L".yml"
#define FILE_MOTIONTRACK_EXT L".mtd"

// save recognizer: folder suffixes
#define FILE_BRIGHTNESS_SUFFIX L"_BRI"
#define FILE_COLOR_SUFFIX L"_COL"
#define FILE_GESTURE_SUFFIX L"_GES"
#define FILE_HAAR_SUFFIX L"_APP"
#define FILE_MOTION_SUFFIX L"_MOT"
#define FILE_SHAPE_SUFFIX L"_SHP"
#define FILE_SIFT_SUFFIX L"_SIF"

// filter IDs (also used as combo box indices)
#define COLOR_FILTER		0
#define SHAPE_FILTER		1
#define BRIGHTNESS_FILTER	2
#define SIFT_FILTER			3
#define ADABOOST_FILTER		4
#define MOTION_FILTER		5
#define GESTURE_FILTER		6

#define APP_CLASS L"Eyepatch"
#define FILTER_CREATE_CLASS L"VideoMarkup"
#define FILTER_COMPOSE_CLASS L"FilterComposer"

// Custom window messages
#define WM_ADD_CUSTOM_FILTER   (WM_APP+1)
#define WM_ADD_STD_FILTER      (WM_APP+2)
#define WM_ADD_DATA_REDUCER    (WM_APP+3)
#define WM_ADD_OUTPUT_SINK     (WM_APP+4)
#define WM_LOAD_FILTER         (WM_APP+5)
#define WM_SET_THRESHOLD       (WM_APP+6)

// Classifier type for standard built-in classifiers
#define FILTER_BUILTIN 10001

// Number of classifier types in the system
#define NUM_FILTERS 7

// listview group IDs
typedef enum {
    GROUPID_POSSAMPLES = 0,
    GROUPID_NEGSAMPLES,
	GROUPID_MOTIONSAMPLES,
    GROUPID_RANGESAMPLES,
    GROUPID_TRASH
} ListviewGroupId;

// eyepatch modes
typedef enum {
    EYEPATCHMODE_CREATEFILTERS = 0,
    EYEPATCHMODE_RUNFILTERS
} EyepatchMode;
