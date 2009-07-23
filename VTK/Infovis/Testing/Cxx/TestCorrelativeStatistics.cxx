/*
 * Copyright 2008 Sandia Corporation.
 * Under the terms of Contract DE-AC04-94AL85000, there is a non-exclusive
 * license for use of this work by or on behalf of the
 * U.S. Government. Redistribution and use in source and binary forms, with
 * or without modification, are permitted provided that this Notice and any
 * statement of authorship are reproduced on all copies.
 */
// .SECTION Thanks
// Thanks to Philippe Pebay from Sandia National Laboratories 
// for implementing this test.

#include "vtkDataObjectCollection.h"
#include "vtkDoubleArray.h"
#include "vtkStringArray.h"
#include "vtkTable.h"
#include "vtkCorrelativeStatistics.h"

//=============================================================================
int TestCorrelativeStatistics( int, char *[] )
{
  int testStatus = 0;

  double mingledData[] = 
    {
      46,
      45,
      47,
      49,
      46,
      47,
      46,
      46,
      47,
      46,
      47,
      49,
      49,
      49,
      47,
      45,
      50,
      50,
      46,
      46,
      51,
      50,
      48,
      48,
      52,
      54,
      48,
      47,
      52,
      52,
      49,
      49,
      53,
      54,
      50,
      50,
      53,
      54,
      50,
      52,
      53,
      53,
      50,
      51,
      54,
      54,
      49,
      49,
      52,
      52,
      50,
      51,
      52,
      52,
      49,
      47,
      48,
      48,
      48,
      50,
      46,
      48,
      47,
      47,
    };
  int nVals1 = 32;

  vtkDoubleArray* dataset1Arr = vtkDoubleArray::New();
  dataset1Arr->SetNumberOfComponents( 1 );
  dataset1Arr->SetName( "Metric 0" );

  vtkDoubleArray* dataset2Arr = vtkDoubleArray::New();
  dataset2Arr->SetNumberOfComponents( 1 );
  dataset2Arr->SetName( "Metric 1" );

  vtkDoubleArray* dataset3Arr = vtkDoubleArray::New();
  dataset3Arr->SetNumberOfComponents( 1 );
  dataset3Arr->SetName( "Metric 2" );

  for ( int i = 0; i < nVals1; ++ i )
    {
    int ti = i << 1;
    dataset1Arr->InsertNextValue( mingledData[ti] );
    dataset2Arr->InsertNextValue( mingledData[ti + 1] );
    dataset3Arr->InsertNextValue( -1. );
    }

  vtkTable* datasetTable1 = vtkTable::New();
  datasetTable1->AddColumn( dataset1Arr );
  dataset1Arr->Delete();
  datasetTable1->AddColumn( dataset2Arr );
  dataset2Arr->Delete();
  datasetTable1->AddColumn( dataset3Arr );
  dataset3Arr->Delete();

  int nMetricPairs = 3;
  vtkStdString columnPairs[] = 
    { 
      "Metric 0", "Metric 1", // First pair
      "Metric 1", "Metric 0", // Second pair
      "Metric 2", "Metric 1"  // Third pair
    };

  // Reference values
  double means1[] = { 49.2188, 49.5 }; // For metrics 0 and 1, respectively
  double covariance1[] = { 5.98286, 7.54839, 6.14516 }; // Covariance matrix of metrics 0 and 1
  double threshold = 4.;

  vtkCorrelativeStatistics* cs1 = vtkCorrelativeStatistics::New();
  cs1->SetInput( vtkStatisticsAlgorithm::INPUT_DATA, datasetTable1 );
  vtkTable* outputData1 = cs1->GetOutput( vtkStatisticsAlgorithm::OUTPUT_DATA );
  vtkTable* outputMeta1 = cs1->GetOutput( vtkStatisticsAlgorithm::OUTPUT_MODEL );

  datasetTable1->Delete();

  // Select Column Pairs of Interest ( Learn Mode ) 
  cs1->AddColumnPair( "Metric 0", "Metric 1" ); // A valid pair
  cs1->AddColumnPair( "Metric 1", "Metric 0" ); // The same valid pair, just reversed
  cs1->AddColumnPair( "Metric 2", "Metric 1" ); // Another valid pair
  for ( int i = 0; i< nMetricPairs; ++ i )
    {  // Try to add all valid pairs once more
    cs1->AddColumnPair( columnPairs[2 * i], columnPairs[ 2 * i + 1] );
    }
  cs1->AddColumnPair( "Metric 1", "Metric 3" ); // An invalid pair

  // Test Learn Mode 
  cs1->SetLearnOption( true );
  cs1->SetDeriveOption( true );
  cs1->SetAssessOption( false );
  cs1->Update();

  for ( vtkIdType r = 0; r < outputMeta1->GetNumberOfRows(); ++ r )
    {
    cout << "   "
         << outputMeta1->GetColumnName( 0 )
         << "="
         << outputMeta1->GetValue( 0, 0 ).ToString();

    // Variable names
    cout << ", (X, Y) = ("
         << outputMeta1->GetValue( r, 1 ).ToString()
         << ", "
         << outputMeta1->GetValue( r, 2 ).ToString()
         << ")";

    // Means
    for ( int i = 3; i < 5; ++ i )
      {
      cout << ", "
           << outputMeta1->GetColumnName( i )
           << "="
           << outputMeta1->GetValue( r, i ).ToDouble();
      }

    // Variances and covariance
    for ( int i = 8; i < 11; ++ i )
      {
      cout << ", "
           << outputMeta1->GetColumnName( i )
           << "="
           << outputMeta1->GetValue( r, i ).ToDouble();
      }

    if ( outputMeta1->GetValueByName( r,  "Linear Correlation" ).ToString() == vtkStdString( "valid" ) )
      {
      cout << "\n   Y = "
           << outputMeta1->GetValueByName( r, "Slope Y/X" ).ToDouble()
           << " * X + "
           << outputMeta1->GetValueByName( r, "Intersect Y/X" ).ToDouble()
           << ", X = "
           << outputMeta1->GetValueByName( r, "Slope X/Y" ).ToDouble()
           << " * Y + "
           << outputMeta1->GetValueByName( r, "Intersect X/Y" ).ToDouble()
           << ", corr. coeff.: "
           << outputMeta1->GetValueByName( r, "Pearson r" ).ToDouble()
           << "\n";
      }
    else
      {
      cout << "\n   Degenerate input, linear correlation was not calculated.\n";
      }
    }

  // Select Column Pairs of Interest ( Assess Mode ) 
  cs1->ResetColumnPairs(); // Clear existing pairs
  cs1->AddColumnPair( columnPairs[0], columnPairs[1] ); // A valid pair

  // Test Assess Mode 
  cout << "## Searching for outliers with respect to this bivariate Gaussian distribution:\n"
       << "   (X, Y) = ("
       << columnPairs[0]
       << ", "
       << columnPairs[1]
       << "), mean=("
       << means1[0]
       << ", "
       << means1[1]
       << "), covariance=["
       << covariance1[0]
       << ", "
       << covariance1[2]
       << " ; "
       << covariance1[2]
       << ", "
       << covariance1[1]
       << "], Squared Mahalanobis > "
       << threshold
       << "\n";

  vtkTable* paramsTable = vtkTable::New();
  paramsTable->ShallowCopy( outputMeta1 );
  paramsTable->SetValueByName( 0, "Mean X", means1[0] );
  paramsTable->SetValueByName( 0, "Mean Y", means1[1] );
  paramsTable->SetValueByName( 0, "Variance X", covariance1[0] );
  paramsTable->SetValueByName( 0, "Variance Y", covariance1[1] );
  paramsTable->SetValueByName( 0, "Covariance", covariance1[2] );
  
  cs1->SetInput( vtkStatisticsAlgorithm::INPUT_MODEL, paramsTable );

  // Test Assess only (Do not recalculate nor rederive a model)
  cs1->SetLearnOption( false );
  cs1->SetDeriveOption( false );
  cs1->SetAssessOption( true );
  cs1->Update();

  int nOutliers = 0;
  int tableIdx[] = { 0, 1, 3 };
  cout << "   Found the following outliers:\n";
  for ( int i = 0; i < 3; ++ i )
    {
    cout << "   "
         << outputData1->GetColumnName( tableIdx[i] );
    }
  cout << "\n";

  for ( vtkIdType r = 0; r < outputData1->GetNumberOfRows(); ++ r )
    {
    if ( outputData1->GetValue( r, tableIdx[2] ).ToDouble() > threshold )
      {
      ++ nOutliers;

      for ( int i = 0; i < 3; ++ i )
        {
        cout << "     "
             << outputData1->GetValue( r,  tableIdx[i] ).ToDouble()
             << "    ";
        }
      cout << "\n";
      }
    }

  if ( nOutliers != 3 )
    {
    vtkGenericWarningMacro("Expected 3 outliers, found " << nOutliers << ".");
    testStatus = 1;
    }

  // Clean up
  paramsTable->Delete();

  // Test with a slight variation of initial data set (to test model aggregation)
  int nVals2 = 32;

  vtkDoubleArray* dataset4Arr = vtkDoubleArray::New();
  dataset4Arr->SetNumberOfComponents( 1 );
  dataset4Arr->SetName( "Metric 0" );

  vtkDoubleArray* dataset5Arr = vtkDoubleArray::New();
  dataset5Arr->SetNumberOfComponents( 1 );
  dataset5Arr->SetName( "Metric 1" );

  vtkDoubleArray* dataset6Arr = vtkDoubleArray::New();
  dataset6Arr->SetNumberOfComponents( 1 );
  dataset6Arr->SetName( "Metric 2" );

  for ( int i = 0; i < nVals2; ++ i )
    {
    int ti = i << 1;
    dataset4Arr->InsertNextValue( mingledData[ti] + 1. );
    dataset5Arr->InsertNextValue( mingledData[ti + 1] );
    dataset6Arr->InsertNextValue( 1. );
    }

  vtkTable* datasetTable2 = vtkTable::New();
  datasetTable2->AddColumn( dataset4Arr );
  dataset4Arr->Delete();
  datasetTable2->AddColumn( dataset5Arr );
  dataset5Arr->Delete();
  datasetTable2->AddColumn( dataset6Arr );
  dataset6Arr->Delete();

  vtkCorrelativeStatistics* cs2 = vtkCorrelativeStatistics::New();
  cs2->SetInput( vtkStatisticsAlgorithm::INPUT_DATA, datasetTable2 );
  vtkTable* outputMeta2 = cs2->GetOutput( vtkStatisticsAlgorithm::OUTPUT_MODEL );

  datasetTable2->Delete();

  // Select all column pairs as pairs of interest
  for ( int i = 0; i< nMetricPairs; ++ i )
    {  // Add all valid pairs
    cs2->AddColumnPair( columnPairs[2 * i], columnPairs[ 2 * i + 1] );
    }

  // Update with Learn option only
  cs2->SetLearnOption( true );
  cs2->SetDeriveOption( false );
  cs2->SetAssessOption( false );
  cs2->Update();

  cout << "\n## Calculated the following statistics for second data set:\n";

  for ( vtkIdType r = 0; r < outputMeta2->GetNumberOfRows(); ++ r )
    {
    cout << "   ";
    for ( int i = 0; i < outputMeta2->GetNumberOfColumns(); ++ i )
      {
      cout << outputMeta2->GetColumnName( i )
           << "="
           << outputMeta2->GetValue( r, i ).ToString()
           << "  ";
      }
    cout << "\n";
    }

  // Now build a data object collection of the two obtained models
  vtkDataObjectCollection* doc = vtkDataObjectCollection::New();
  doc->AddItem( outputMeta1 );
  doc->AddItem( outputMeta2 );

  // And calculate the aggregated minimal statistics of the two models
  vtkCorrelativeStatistics* cs = vtkCorrelativeStatistics::New();
  vtkTable* aggregated = vtkTable::New();
  cs->Aggregate( doc, aggregated );

  // Finally, calculate the derived statistics of the aggregated model
  cs2->SetInput( vtkStatisticsAlgorithm::INPUT_MODEL, aggregated );
  cs2->SetLearnOption( false );
  cs2->SetDeriveOption( true ); 
  cs2->SetAssessOption( false );
  cs2->Update();

  cout << "\n## Calculated the following statistics for aggregated (first + second) data set:\n";

  for ( vtkIdType r = 0; r < outputMeta2->GetNumberOfRows(); ++ r )
    {
    cout << "   "
         << outputMeta2->GetColumnName( 0 )
         << "="
         << outputMeta2->GetValue( 0, 0 ).ToString();

    // Variable names
    cout << ", (X, Y) = ("
         << outputMeta2->GetValue( r, 1 ).ToString()
         << ", "
         << outputMeta2->GetValue( r, 2 ).ToString()
         << ")";

    // Means
    for ( int i = 3; i < 5; ++ i )
      {
      cout << ", "
           << outputMeta2->GetColumnName( i )
           << "="
           << outputMeta2->GetValue( r, i ).ToDouble();
      }

    // Variances and covariance
    for ( int i = 8; i < 11; ++ i )
      {
      cout << ", "
           << outputMeta2->GetColumnName( i )
           << "="
           << outputMeta2->GetValue( r, i ).ToDouble();
      }

    if ( outputMeta2->GetValueByName( r,  "Linear Correlation" ).ToString() == vtkStdString( "valid" ) )
      {
      cout << "\n   Y = "
           << outputMeta2->GetValueByName( r, "Slope Y/X" ).ToDouble()
           << " * X + "
           << outputMeta2->GetValueByName( r, "Intersect Y/X" ).ToDouble()
           << ", X = "
           << outputMeta2->GetValueByName( r, "Slope X/Y" ).ToDouble()
           << " * Y + "
           << outputMeta2->GetValueByName( r, "Intersect X/Y" ).ToDouble()
           << ", corr. coeff.: "
           << outputMeta2->GetValueByName( r, "Pearson r" ).ToDouble()
           << "\n";
      }
    else
      {
      cout << "\n   Degenerate input, linear correlation was not calculated.\n";
      }
    }

  // Clean up
  doc->Delete();
  cs->Delete();
  cs1->Delete();
  cs2->Delete();

  return testStatus;
}
