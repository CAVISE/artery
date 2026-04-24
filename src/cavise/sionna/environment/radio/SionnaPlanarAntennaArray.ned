package cavise.sionna.environment.radio;

import cavise.sionna.environment.radio.SionnaAntennaArray;

module SionnaPlanarAntennaArray extends SionnaAntennaArray
{
    parameters:
        @class(SionnaPlanarAntennaArray);
        int numRows;
        int numCols;
        double verticalSpacing = default(0.5);
        double horizontalSpacing = default(0.5);
        string pattern;
        string polarization = default("");
        string polarizationModel = default("");
}
