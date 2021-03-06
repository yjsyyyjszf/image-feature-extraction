/* 
   Calculate features derived from the gradient estimated in each voxel, and 
   masked with mask image.
   The gradient is estimated by convolution with 1'st order derivative operators.
   These appear to be based on central differences, see 
   http://www.itk.org/Doxygen48/html/classitk_1_1DerivativeOperator.html

   The calculated features are 
   * Gradient magnitude
*/
#include <string>
#include <vector>

#include "tclap/CmdLine.h"

#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkGradientMagnitudeImageFilter.h"
#include "itkMaskImageFilter.h"

#include "ife/Util/Path.h"

const std::string VERSION("0.1");
const std::string OUT_FILE_TYPE(".nii.gz");

int main(int argc, char *argv[]) {
  // Commandline parsing
  TCLAP::CmdLine cmd("Calculate gradient based features.", ' ', VERSION);

  // We need a single image
  TCLAP::ValueArg<std::string> 
    imageArg("i", 
	     "image", 
	     "Path to image.",
	     true,
	     "",
	     "path", 
	     cmd);

  // We need a single mask
  TCLAP::ValueArg<std::string> 
    maskArg("m", 
	    "mask", 
	    "Path to mask. Must match image dimensions.",
	    true,
	    "",
	    "path", 
	    cmd);
  
  // We need a directory for storing the resulting images
  TCLAP::ValueArg<std::string> 
    outDirArg("o", 
	      "outdir", 
	      "Path to output directory",
	      true, 
	      "", 
	      "path", 
	      cmd);
  
  // We can use a prefix to generate filenames
  TCLAP::ValueArg<std::string> 
    prefixArg("p", 
	      "prefix", 
	      "Prefix to use for output filenames",
	      false, 
	      "gradient_", 
	      "string", 
	      cmd);

  
  try {
    cmd.parse(argc, argv);
  } catch(TCLAP::ArgException &e) {
    std::cerr << "Error : " << e.error() 
	      << " for arg " << e.argId() 
	      << std::endl;
    return EXIT_FAILURE;
  }

  // Store the arguments
  std::string imagePath( imageArg.getValue() );
  std::string maskPath( maskArg.getValue() );
  std::string outDirPath( outDirArg.getValue() );
  std::string prefix( prefixArg.getValue() );

  //// Commandline parsing is done ////


  // Some common values/types that are always used.
  const unsigned int Dimension = 3;

  // TODO: Need to consider which pixeltype to use
  typedef float PixelType;
  typedef itk::Image< PixelType, Dimension >  ImageType;
  typedef itk::ImageFileReader< ImageType > ReaderType;
    
  // Setup the readers
  ReaderType::Pointer imageReader = ReaderType::New();
  imageReader->SetFileName( imagePath );

  ReaderType::Pointer maskReader = ReaderType::New();
  maskReader->SetFileName( maskPath );

  // Setup the gradient magnitude filter
  typedef itk::GradientMagnitudeImageFilter< ImageType, ImageType > FilterType;
  FilterType::Pointer filter = FilterType::New();
  filter->SetInput( imageReader->GetOutput() );

  // Setup the mask filter 
  typedef itk::MaskImageFilter< ImageType, ImageType, ImageType > MaskFilterType;
  MaskFilterType::Pointer maskFilter = MaskFilterType::New();
  maskFilter->SetInput1( filter->GetOutput() );
  maskFilter->SetInput2( maskReader->GetOutput() );
  
  // Setup the writer
  typedef itk::ImageFileWriter< ImageType >  WriterType;
  WriterType::Pointer writer =  WriterType::New();
  writer->SetInput( maskFilter->GetOutput() );

  // Base file name for output images
  std::string baseFileName = Path::join( outDirPath, prefix );

  // Create a filename
  std::string outFile = baseFileName + "GradientMagnitude" + OUT_FILE_TYPE;
  writer->SetFileName( outFile );
  try {
    writer->Update();
  }
  catch ( itk::ExceptionObject &e ) {
    std::cerr << "Failed to process." << std::endl
	      << "Image: " << imagePath << std::endl
	      << "Mask: " << maskPath << std::endl
	      << "Base file name: " << baseFileName << std::endl
	      << "ExceptionObject: " << e << std::endl;
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}
