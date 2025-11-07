#include <stdio.h>
#include <math.h>
#include <string.h>

#define MAX_STATIONS 20
#define PI 3.14159265358979323846

// Structure to store bearing data
struct Bearing {
    double degrees;
    double minutes;
    double seconds;
    double decimal_degrees;
    char quadrant[4];
};

// Structure for station data
struct Station {
    char name[10];
    struct Bearing fore_bearing;
    struct Bearing corrected_bearing;
    double interior_angle;
    double corrected_interior_angle;
    double line_length;
    double latitude;
    double departure;
    double northing;
    double southing;
    double easting;
    double westing;
    double corrected_northing;
    double corrected_southing;
    double corrected_easting;
    double corrected_westing;
    double independent_northing;
    double independent_easting;
    double angle_correction;
};

// Function prototypes
double toDecimalDegrees(double deg, double min, double sec);
void toDegreesMinutesSeconds(double decimal, double *deg, double *min, double *sec);
void calculateQuadrant(double bearing, char *quadrant);
void calculateWCBfromQCB(struct Bearing *bearing);
void calculateQCBfromWCB(struct Bearing *bearing);
void calculateLatitudeDeparture(struct Station *station);
void calculateConsecutiveCoordinates(struct Station stations[], int num_stations);
void applyBowditchCorrection(struct Station stations[], int num_stations, double sum_lat, double sum_dep, double total_length);
void calculateIndependentCoordinates(struct Station stations[], int num_stations);
void calculateWCBFromInteriorAngles(struct Station stations[], int num_stations, int start_index, int bearing_type, int traverse_direction);
void applyAngleCorrection(struct Station stations[], int num_stations);
void displayResults(struct Station stations[], int num_stations, int input_type, double sum_lat, double sum_dep, double total_length);

int main() {
    struct Station stations[MAX_STATIONS];
    int num_stations, input_type;
    double total_length = 0;
    
    printf("=== COMPREHENSIVE TRAVERSE CALCULATIONS ===\n\n");
    
    // Get number of stations
    printf("Enter number of stations: ");
    scanf("%d", &num_stations);
    
    if (num_stations > MAX_STATIONS || num_stations < 3) {
        printf("Error: Number of stations must be between 3 and %d\n", MAX_STATIONS);
        return 1;
    }
    
    // Input type selection
    printf("\nEnter input type:\n");
    printf("1 - Whole Circle Bearing (WCB)\n");
    printf("2 - Quadrantal Bearing System (QBS)\n");
    printf("3 - Interior Angles\n");
    printf("Choice: ");
    scanf("%d", &input_type);
    
    // Input station data
    printf("\n=== ENTER STATION DATA ===\n");
    for (int i = 0; i < num_stations; i++) {
        printf("\nStation %d:\n", i + 1);
        printf("Enter station name: ");
        scanf("%s", stations[i].name);
        printf("Enter line length (m): ");
        scanf("%lf", &stations[i].line_length);
        total_length += stations[i].line_length;
        
        if (input_type == 1) {
            // WCB input
            printf("Enter Fore Bearing (WCB - Degrees Minutes Seconds):\n");
            printf("Degrees: ");
            scanf("%lf", &stations[i].fore_bearing.degrees);
            printf("Minutes: ");
            scanf("%lf", &stations[i].fore_bearing.minutes);
            printf("Seconds: ");
            scanf("%lf", &stations[i].fore_bearing.seconds);
            
            stations[i].fore_bearing.decimal_degrees = toDecimalDegrees(
                stations[i].fore_bearing.degrees,
                stations[i].fore_bearing.minutes,
                stations[i].fore_bearing.seconds
            );
            
            calculateQuadrant(stations[i].fore_bearing.decimal_degrees, 
                            stations[i].fore_bearing.quadrant);
            
        } else if (input_type == 2) {
            // QBS input
            printf("Enter Fore Bearing (QBS):\n");
            printf("Degrees: ");
            scanf("%lf", &stations[i].fore_bearing.degrees);
            printf("Minutes: ");
            scanf("%lf", &stations[i].fore_bearing.minutes);
            printf("Seconds: ");
            scanf("%lf", &stations[i].fore_bearing.seconds);
            printf("Quadrant (NE, NW, SE, SW): ");
            scanf("%s", stations[i].fore_bearing.quadrant);
            
            stations[i].fore_bearing.decimal_degrees = toDecimalDegrees(
                stations[i].fore_bearing.degrees,
                stations[i].fore_bearing.minutes,
                stations[i].fore_bearing.seconds
            );
            
            calculateWCBfromQCB(&stations[i].fore_bearing);
            calculateQuadrant(stations[i].fore_bearing.decimal_degrees, 
                            stations[i].fore_bearing.quadrant);
            
        } else if (input_type == 3) {
            // Interior Angles input
            printf("Enter Interior Angle (Degrees Minutes Seconds):\n");
            printf("Degrees: ");
            scanf("%lf", &stations[i].fore_bearing.degrees);
            printf("Minutes: ");
            scanf("%lf", &stations[i].fore_bearing.minutes);
            printf("Seconds: ");
            scanf("%lf", &stations[i].fore_bearing.seconds);
            
            stations[i].interior_angle = toDecimalDegrees(
                stations[i].fore_bearing.degrees,
                stations[i].fore_bearing.minutes,
                stations[i].fore_bearing.seconds
            );
            
            // Initialize bearing fields to zero
            stations[i].fore_bearing.decimal_degrees = 0;
            strcpy(stations[i].fore_bearing.quadrant, "NE");
        }
    }
    
    // SPECIAL PROCESSING FOR INTERIOR ANGLES
    if (input_type == 3) {
        // Step 1: Calculate sum of interior angles and apply correction
        applyAngleCorrection(stations, num_stations);
        
        // Step 2: Ask user for one known bearing
        printf("\n=== ENTER KNOWN BEARING FOR REFERENCE ===\n");
        int ref_station;
        printf("Enter station number for which you know the bearing (1 to %d): ", num_stations);
        scanf("%d", &ref_station);
        ref_station--; // Convert to zero-based index
        
        if (ref_station < 0 || ref_station >= num_stations) {
            printf("Invalid station number! Using station 1 as reference.\n");
            ref_station = 0;
        }
        
        int ref_bearing_type;
        printf("Enter bearing type for reference station:\n");
        printf("1 - Whole Circle Bearing (WCB)\n");
        printf("2 - Quadrantal Bearing System (QBS)\n");
        printf("Choice: ");
        scanf("%d", &ref_bearing_type);
        
        if (ref_bearing_type == 1) {
            // WCB input for reference station
            printf("Enter WCB for station %s (Degrees Minutes Seconds):\n", stations[ref_station].name);
            printf("Degrees: ");
            scanf("%lf", &stations[ref_station].fore_bearing.degrees);
            printf("Minutes: ");
            scanf("%lf", &stations[ref_station].fore_bearing.minutes);
            printf("Seconds: ");
            scanf("%lf", &stations[ref_station].fore_bearing.seconds);
            
            stations[ref_station].fore_bearing.decimal_degrees = toDecimalDegrees(
                stations[ref_station].fore_bearing.degrees,
                stations[ref_station].fore_bearing.minutes,
                stations[ref_station].fore_bearing.seconds
            );
            
            calculateQuadrant(stations[ref_station].fore_bearing.decimal_degrees, 
                            stations[ref_station].fore_bearing.quadrant);
            
        } else if (ref_bearing_type == 2) {
            // QBS input for reference station
            printf("Enter QBS for station %s:\n", stations[ref_station].name);
            printf("Degrees: ");
            scanf("%lf", &stations[ref_station].fore_bearing.degrees);
            printf("Minutes: ");
            scanf("%lf", &stations[ref_station].fore_bearing.minutes);
            printf("Seconds: ");
            scanf("%lf", &stations[ref_station].fore_bearing.seconds);
            printf("Quadrant (NE, NW, SE, SW): ");
            scanf("%s", stations[ref_station].fore_bearing.quadrant);
            
            stations[ref_station].fore_bearing.decimal_degrees = toDecimalDegrees(
                stations[ref_station].fore_bearing.degrees,
                stations[ref_station].fore_bearing.minutes,
                stations[ref_station].fore_bearing.seconds
            );
            
            calculateWCBfromQCB(&stations[ref_station].fore_bearing);
            calculateQuadrant(stations[ref_station].fore_bearing.decimal_degrees, 
                            stations[ref_station].fore_bearing.quadrant);
        }
        
        // Step 3: Get traverse direction from user
        int traverse_direction;
        printf("\n=== SELECT TRAVERSE DIRECTION ===\n");
        printf("1 - Clockwise (Right-handed traverse)\n");
        printf("2 - Anti-clockwise (Left-handed traverse)\n");
        printf("Choice: ");
        scanf("%d", &traverse_direction);
        
        // Step 4: Calculate WCB for all stations using corrected interior angles
        calculateWCBFromInteriorAngles(stations, num_stations, ref_station, ref_bearing_type, traverse_direction);
    }
    
    // Calculate latitude and departure for all stations
    for (int i = 0; i < num_stations; i++) {
        calculateLatitudeDeparture(&stations[i]);
    }
    
    // Calculate sums for error determination
    double sum_lat = 0, sum_dep = 0;
    for (int i = 0; i < num_stations; i++) {
        sum_lat += stations[i].latitude;
        sum_dep += stations[i].departure;
    }
    
    // Calculate corrections
    applyBowditchCorrection(stations, num_stations, sum_lat, sum_dep, total_length);
    calculateConsecutiveCoordinates(stations, num_stations);
    calculateIndependentCoordinates(stations, num_stations);
    
    // DISPLAY RESULTS
    displayResults(stations, num_stations, input_type, sum_lat, sum_dep, total_length);

    printf("\n\nPress Enter to exit...");
    getchar(); // this will take the leftover '\n'
    getchar(); // this will wait for user to press ENTER

    
    return 0;
}

// Display all results with clear table formatting
void displayResults(struct Station stations[], int num_stations, int input_type, double sum_lat, double sum_dep, double total_length) {
    // DISPLAY TABLE 1: TRAVERSE CALCULATIONS
    printf("\n\n====================================================================================================================================================================================================");
    printf("\n                                                                  TRAVERSE CALCULATIONS TABLE");
    printf("\n====================================================================================================================================================================================================\n");
    
    if (input_type == 3) {
        printf("Inst.   | Interior     | Angle        | Corrected   |                     | Line   |           Consecutive Coordinates        |\n");
        printf("Station | Angle        | Correction   | Interior    | WCB          | Quad | Length |---------------------------------------------------|\n");
        printf("        |              |              |             |              |      |        |      Latitude       |        Departure            |\n");
        printf("        |              |              |             |              |      |        |---------------------|-----------------------------|\n");
        printf("        |              |              |             |              |      |        | Northing | Southing | Easting | Westing |\n");
        printf("        |              |              |             |              |      |        |  (Calc)  |  (Calc)  | (Calc)  | (Calc)  |\n");
        printf("--------|--------------|--------------|-------------|--------------|------|--------|----------|----------|---------|---------|\n");
        
        for (int i = 0; i < num_stations; i++) {
            // Convert all angles to DMS
            double orig_deg, orig_min, orig_sec;
            double corr_int_deg, corr_int_min, corr_int_sec;
            double angle_corr_deg, angle_corr_min, angle_corr_sec;
            
            toDegreesMinutesSeconds(stations[i].interior_angle, &orig_deg, &orig_min, &orig_sec);
            toDegreesMinutesSeconds(stations[i].corrected_interior_angle, &corr_int_deg, &corr_int_min, &corr_int_sec);
            toDegreesMinutesSeconds(fabs(stations[i].angle_correction), &angle_corr_deg, &angle_corr_min, &angle_corr_sec);
            
            // Display correction sign
            char corr_sign = (stations[i].angle_correction >= 0) ? '+' : '-';
            
            printf("%-6s  | %3.0f°%2.0f'%5.2f\"| %c%2.0f°%2.0f'%5.2f\"| %3.0f°%2.0f'%5.2f\"| %3.0f°%2.0f'%5.2f\"| %-6s| %6.2f| %8.3f | %8.3f| %7.3f| %7.3f|\n",
                   stations[i].name,
                   orig_deg, orig_min, orig_sec,                    // Original interior angle
                   corr_sign, angle_corr_deg, angle_corr_min, angle_corr_sec,  // Angle correction with sign
                   corr_int_deg, corr_int_min, corr_int_sec,        // Corrected interior angle
                   stations[i].corrected_bearing.degrees, stations[i].corrected_bearing.minutes, stations[i].corrected_bearing.seconds, // Corrected bearing
                   
                   stations[i].corrected_bearing.quadrant,
                   stations[i].line_length,
                   stations[i].northing, stations[i].southing, stations[i].easting, stations[i].westing);  // Calculated values
        }
    } else {
        printf("Inst.   | Given        | Angle       |          |        | Line   |                    Consecutive Coordinates     |\n");
        printf("Station | Angle        | Correction  | WCB      | Quad   | Length |---------------------------------------------------|\n");
        printf("        |              |             |          |        |        |      Latitude       |        Departure              |\n");
        printf("        |              |             |          |        |        |---------------------|-------------------------------|\n");
        printf("        |              |             |          |        |        | Northing | Southing | Easting | Westing | \n");
        printf("        |              |             |          |        |        |  (Calc)  |  (Calc)  | (Calc)  | (Calc)  | \n");
        printf("--------|--------------|-------------|----------|--------|--------|----------|----------|---------|---------|\n");
        
        for (int i = 0; i < num_stations; i++) {
            // Convert angle correction to DMS
            double angle_corr_deg, angle_corr_min, angle_corr_sec;
            toDegreesMinutesSeconds(fabs(stations[i].angle_correction), &angle_corr_deg, &angle_corr_min, &angle_corr_sec);
            
            // Display correction sign
            char corr_sign = (stations[i].angle_correction >= 0) ? '+' : '-';
            
            printf("%-6s  | %3.0f°%2.0f'%5.2f\" | %c%2.0f°%2.0f'%5.2f\" | %3.0f°%2.0f'%5.2f\" | %7.2f° | %-6s | %6.2f | %8.3f  | %8.3f  | %7.3f | %7.3f |\n",
                   stations[i].name,
                   stations[i].fore_bearing.degrees, stations[i].fore_bearing.minutes, stations[i].fore_bearing.seconds,
                   corr_sign, angle_corr_deg, angle_corr_min, angle_corr_sec,
                   stations[i].corrected_bearing.degrees, stations[i].corrected_bearing.minutes, stations[i].corrected_bearing.seconds,
                   stations[i].corrected_bearing.decimal_degrees,
                   stations[i].corrected_bearing.quadrant,
                   stations[i].line_length,
                   stations[i].northing, stations[i].southing, stations[i].easting, stations[i].westing); // Calculated values
                
        }
    }
    
    // DISPLAY TABLE 2: CORRECTIONS TABLE
    printf("\n\n========================================================================================");
    printf("\n                             CORRECTIONS TABLE");
    printf("\n========================================================================================\n");
    printf("Inst.   |        Correction in Consecutive Coordinates       |\n");
    printf("Station |-----------------------------------------------------|\n");
    printf("        |  Northing  |  Southing  |  Easting   |  Westing    |\n");
    printf("--------|------------|------------|------------|-------------|\n");
    
    for (int i = 0; i < num_stations; i++) {
        double northing_corr = stations[i].corrected_northing - stations[i].northing;
        double southing_corr = stations[i].corrected_southing - stations[i].southing;
        double easting_corr = stations[i].corrected_easting - stations[i].easting;
        double westing_corr = stations[i].corrected_westing - stations[i].westing;
        
        printf("%-6s  | %10.3f | %10.3f | %10.3f | %10.3f  |\n",
               stations[i].name,
               northing_corr,
               southing_corr,
               easting_corr,
               westing_corr);
    }
    
    // DISPLAY TABLE 3: CORRECTED COORDINATES
    printf("\n\n============================================================================================================");
    printf("\n                         CORRECTED CONSECUTIVE COORDINATES & INDEPENDENT COORDINATES");
    printf("\n============================================================================================================\n");
    printf("Inst.   |      Corrected Consecutive Coordinates     |   Independent Coordinates  |\n");
    printf("Station |--------------------------------------------|----------------------------|\n");
    printf("        | Northing | Southing | Easting | Westing    |  Northing   |   Easting    |\n");
    printf("--------|----------|----------|---------|------------|-------------|-------------|\n");
    
    for (int i = 0; i < num_stations; i++) {
        printf("%-6s  | %8.3f | %8.3f | %7.3f | %8.3f  | %11.3f | %11.3f  |\n",
               stations[i].name,
               stations[i].corrected_northing,
               stations[i].corrected_southing,
               stations[i].corrected_easting,
               stations[i].corrected_westing,
               stations[i].independent_northing,
               stations[i].independent_easting);
    }
    
    // ERROR SUMMARY
    double linear_error = sqrt(sum_lat * sum_lat + sum_dep * sum_dep);
    double accuracy_ratio = total_length / linear_error;
    double error_direction = atan2(sum_dep, sum_lat) * 180.0 / PI;
    if (error_direction < 0) error_direction += 360.0;
    
    // Convert error direction to DMS
    double error_deg, error_min, error_sec;
    toDegreesMinutesSeconds(error_direction, &error_deg, &error_min, &error_sec);
    
    printf("\n\n=== ERROR SUMMARY ===\n");
    printf("Sum of Latitude:        %12.6f m\n", sum_lat);
    printf("Sum of Departure:       %12.6f m\n", sum_dep);
    printf("Linear Error:           %12.6f m\n", linear_error);
    printf("Error Direction:        %3.0f°%2.0f'%5.2f\"\n", error_deg, error_min, error_sec);
    printf("Total Traverse Length:  %12.3f m\n", total_length);
    printf("Accuracy Ratio:         1:%.0f\n", accuracy_ratio);
}

// Convert degrees, minutes, seconds to decimal degrees
double toDecimalDegrees(double deg, double min, double sec) {
    return deg + min/60.0 + sec/3600.0;
}

// Convert decimal degrees to degrees, minutes, seconds
void toDegreesMinutesSeconds(double decimal, double *deg, double *min, double *sec) {
    *deg = floor(decimal);
    double remainder = (decimal - *deg) * 60.0;
    *min = floor(remainder);
    *sec = (remainder - *min) * 60.0;
}

// Calculate quadrant from WCB
void calculateQuadrant(double bearing, char *quadrant) {
    if (bearing >= 0 && bearing < 90) {
        strcpy(quadrant, "NE");
    } else if (bearing >= 90 && bearing < 180) {
        strcpy(quadrant, "SE");
    } else if (bearing >= 180 && bearing < 270) {
        strcpy(quadrant, "SW");
    } else {
        strcpy(quadrant, "NW");
    }
}

// Convert QCB to WCB
void calculateWCBfromQCB(struct Bearing *bearing) {
    double angle = bearing->decimal_degrees;
    
    if (strcmp(bearing->quadrant, "NE") == 0) {
        bearing->decimal_degrees = angle;
    } else if (strcmp(bearing->quadrant, "SE") == 0) {
        bearing->decimal_degrees = 180.0 - angle;
    } else if (strcmp(bearing->quadrant, "SW") == 0) {
        bearing->decimal_degrees = 180.0 + angle;
    } else if (strcmp(bearing->quadrant, "NW") == 0) {
        bearing->decimal_degrees = 360.0 - angle;
    }
}

// Convert WCB to QCB
void calculateQCBfromWCB(struct Bearing *bearing) {
    double wcb = bearing->decimal_degrees;
    double angle;
    
    if (wcb >= 0 && wcb < 90) {
        strcpy(bearing->quadrant, "NE");
        angle = wcb;
    } else if (wcb >= 90 && wcb < 180) {
        strcpy(bearing->quadrant, "SE");
        angle = 180.0 - wcb;
    } else if (wcb >= 180 && wcb < 270) {
        strcpy(bearing->quadrant, "SW");
        angle = wcb - 180.0;
    } else {
        strcpy(bearing->quadrant, "NW");
        angle = 360.0 - wcb;
    }
    
    toDegreesMinutesSeconds(angle, &bearing->degrees, &bearing->minutes, &bearing->seconds);
}

// Calculate latitude and departure
void calculateLatitudeDeparture(struct Station *station) {
    double bearing_rad = station->fore_bearing.decimal_degrees * PI / 180.0;
    station->latitude = station->line_length * cos(bearing_rad);
    station->departure = station->line_length * sin(bearing_rad);
    
    // Calculate northing/southing and easting/westing
    if (station->latitude >= 0) {
        station->northing = station->latitude;
        station->southing = 0;
    } else {
        station->northing = 0;
        station->southing = -station->latitude;
    }
    
    if (station->departure >= 0) {
        station->easting = station->departure;
        station->westing = 0;
    } else {
        station->easting = 0;
        station->westing = -station->departure;
    }
}

// Calculate WCB from interior angles with user-selected direction
void calculateWCBFromInteriorAngles(struct Station stations[], int num_stations, int start_index, int bearing_type, int traverse_direction) {
    // Copy the reference bearing to corrected bearing
    stations[start_index].corrected_bearing = stations[start_index].fore_bearing;
    
    int is_clockwise = (traverse_direction == 1);
    
    printf("\nTraverse Direction: %s\n", is_clockwise ? "CLOCKWISE" : "ANTI-CLOCKWISE");
    printf("\n=== BEARING CALCULATIONS ===\n");
    
    // Calculate forward using user-selected direction
    for (int i = (start_index + 1) % num_stations, count = 0; count < num_stations - 1; i = (i + 1) % num_stations, count++) {
        int prev_index = (i - 1 + num_stations) % num_stations;
        
        double prev_wcb = stations[prev_index].corrected_bearing.decimal_degrees;
        double interior_angle = stations[i].corrected_interior_angle;
        double new_wcb;
        
        if (is_clockwise) {
            // Clockwise traverse formula: WCB_next = WCB_previous + 180° - Interior_Angle_next
            new_wcb = prev_wcb + 180.0 - interior_angle;
            printf("Station %s: %.3f° + 180° - %.3f° = %.3f°", 
                   stations[i].name, prev_wcb, interior_angle, new_wcb);
        } else {
            // Anti-clockwise traverse formula: WCB_next = WCB_previous + Interior_Angle_next - 180°
            new_wcb = prev_wcb + interior_angle - 180.0;
            printf("Station %s: %.3f° + %.3f° - 180° = %.3f°", 
                   stations[i].name, prev_wcb, interior_angle, new_wcb);
        }
        
        // Normalize to 0-360 range
        if (new_wcb >= 360.0) {
            new_wcb -= 360.0;
            printf(" → Normalized to: %.3f°", new_wcb);
        }
        if (new_wcb < 0) {
            new_wcb += 360.0;
            printf(" → Normalized to: %.3f°", new_wcb);
        }
        printf("\n");
        
        stations[i].corrected_bearing.decimal_degrees = new_wcb;
        
        toDegreesMinutesSeconds(stations[i].corrected_bearing.decimal_degrees,
                              &stations[i].corrected_bearing.degrees,
                              &stations[i].corrected_bearing.minutes,
                              &stations[i].corrected_bearing.seconds);
        calculateQuadrant(stations[i].corrected_bearing.decimal_degrees, 
                        stations[i].corrected_bearing.quadrant);
        
        // Copy to fore_bearing for latitude/departure calculations
        stations[i].fore_bearing = stations[i].corrected_bearing;
    }
}

// Apply angle correction using (2n-4)*90 formula
void applyAngleCorrection(struct Station stations[], int num_stations) {
    double sum_observed_angles = 0;
    
    // Calculate sum of observed interior angles
    for (int i = 0; i < num_stations; i++) {
        sum_observed_angles += stations[i].interior_angle;
    }
    
    // Theoretical sum for closed traverse: (2n-4)*90 degrees
    double theoretical_sum = (2 * num_stations - 4) * 90.0;
    double angular_error = sum_observed_angles - theoretical_sum;
    double correction_per_station = -angular_error / num_stations;
    
    // Convert to DMS for display
    double theo_deg, theo_min, theo_sec;
    double obs_deg, obs_min, obs_sec;
    double error_deg, error_min, error_sec;
    double corr_deg, corr_min, corr_sec;
    
    toDegreesMinutesSeconds(theoretical_sum, &theo_deg, &theo_min, &theo_sec);
    toDegreesMinutesSeconds(sum_observed_angles, &obs_deg, &obs_min, &obs_sec);
    toDegreesMinutesSeconds(fabs(angular_error), &error_deg, &error_min, &error_sec);
    toDegreesMinutesSeconds(fabs(correction_per_station), &corr_deg, &corr_min, &corr_sec);
    
    char error_sign = (angular_error >= 0) ? '+' : '-';
    char corr_sign = (correction_per_station >= 0) ? '+' : '-';
    
    printf("\n=== ANGLE CORRECTION SUMMARY ===\n");
    printf("Number of Stations (n): %d\n", num_stations);
    printf("Theoretical Sum (2n-4)*90: %3.0f°%2.0f'%5.2f\"\n", theo_deg, theo_min, theo_sec);
    printf("Observed Sum of Angles:     %3.0f°%2.0f'%5.2f\"\n", obs_deg, obs_min, obs_sec);
    printf("Angular Error:              %c%2.0f°%2.0f'%5.2f\"\n", error_sign, error_deg, error_min, error_sec);
    printf("Correction per Station:     %c%2.0f°%2.0f'%5.2f\"\n", corr_sign, corr_deg, corr_min, corr_sec);
    
    // Apply correction to each station
    for (int i = 0; i < num_stations; i++) {
        stations[i].angle_correction = correction_per_station;
        stations[i].corrected_interior_angle = stations[i].interior_angle + correction_per_station;
    }
}

// CORRECTED Bowditch correction function
void applyBowditchCorrection(struct Station stations[], int num_stations, double sum_lat, double sum_dep, double total_length) {
    printf("\n=== APPLYING BOWDITCH CORRECTION ===\n");
    printf("Total Latitude Error (ΣL): %.6f m\n", sum_lat);
    printf("Total Departure Error (ΣD): %.6f m\n", sum_dep);
    printf("Total Traverse Length: %.6f m\n", total_length);
    
    for (int i = 0; i < num_stations; i++) {
        // Calculate corrections using Bowditch Rule
        double proportion = stations[i].line_length / total_length;
        double corr_lat = -sum_lat * proportion;
        double corr_dep = -sum_dep * proportion;
        
        printf("Station %s: Length=%.3f, Proportion=%.6f, LatCorr=%.6f, DepCorr=%.6f\n",
               stations[i].name, stations[i].line_length, proportion, corr_lat, corr_dep);
        
        // Apply corrections - ONLY to either northing OR southing, and either easting OR westing
        if (stations[i].northing > 0) {
            // If we have northing, apply correction to northing only
            stations[i].corrected_northing = stations[i].northing + corr_lat;
            stations[i].corrected_southing = stations[i].southing;
        } else {
            // If we have southing, apply correction to southing only (with sign adjustment)
            stations[i].corrected_northing = stations[i].northing;
            stations[i].corrected_southing = stations[i].southing - corr_lat;
        }
        
        if (stations[i].easting > 0) {
            // If we have easting, apply correction to easting only
            stations[i].corrected_easting = stations[i].easting + corr_dep;
            stations[i].corrected_westing = stations[i].westing;
        } else {
            // If we have westing, apply correction to westing only (with sign adjustment)
            stations[i].corrected_easting = stations[i].easting;
            stations[i].corrected_westing = stations[i].westing - corr_dep;
        }
        
        // Ensure non-negative values (but don't double the correction)
        if (stations[i].corrected_northing < 0) {
            stations[i].corrected_northing = 0;
        }
        if (stations[i].corrected_southing < 0) {
            stations[i].corrected_southing = 0;
        }
        if (stations[i].corrected_easting < 0) {
            stations[i].corrected_easting = 0;
        }
        if (stations[i].corrected_westing < 0) {
            stations[i].corrected_westing = 0;
        }
    }
}

// Calculate consecutive coordinates
void calculateConsecutiveCoordinates(struct Station stations[], int num_stations) {
    // Already calculated in applyBowditchCorrection
}

// Calculate independent coordinates (starting from arbitrary point)
void calculateIndependentCoordinates(struct Station stations[], int num_stations) {
    double start_northing = 1000.0;
    double start_easting = 1000.0;
    
    stations[0].independent_northing = start_northing;
    stations[0].independent_easting = start_easting;
    
    for (int i = 1; i < num_stations; i++) {
        stations[i].independent_northing = stations[i-1].independent_northing + 
                                          stations[i].corrected_northing - stations[i].corrected_southing;
        stations[i].independent_easting = stations[i-1].independent_easting + 
                                         stations[i].corrected_easting - stations[i].corrected_westing;
    }
}

