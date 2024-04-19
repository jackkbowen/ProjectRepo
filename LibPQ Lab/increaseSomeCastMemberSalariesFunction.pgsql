CREATE OR REPLACE FUNCTION
increaseSomeCastMemberSalariesFunction(thePlayTitle VARCHAR(40), theProductionNum INTEGER, maxDailyCost NUMERIC(7,2))
RETURNS NUMERIC(7,2) AS $$

    DECLARE
        totalDailyCost NUMERIC(7,2) := 0;
        castSalary NUMERIC(6,2) := 0;
        dailyTheaterFee NUMERIC(6,2) := 0;
        outOfMoney BOOLEAN := TRUE;


    DECLARE increaseSalaryCursor CURSOR FOR 
        SELECT salaryPerDay
        FROM CastMembers
        WHERE playTitle = thePlayTitle
        AND productionNum = theProductionNum
        ORDER BY salaryPerDay;
        
    BEGIN
        IF maxDailyCost <= 0 THEN 
            RETURN -1;
        END IF;

        SELECT theaterFeePerDay INTO dailyTheaterFee
        FROM Theaters t
        WHERE t.theaterID = (SELECT p.theaterID
                                FROM Productions p
                                WHERE p.playTitle = thePlayTitle
                                AND p.productionNum = theProductionNum);
        
        
        totalDailyCost := dailyTheaterFee;

        OPEN increaseSalaryCursor;

        LOOP
            FETCH increaseSalaryCursor INTO castSalary;
            EXIT WHEN NOT FOUND;

            totalDailyCost := castSalary + totalDailyCost;
        END LOOP;
        
        CLOSE increaseSalaryCursor;

        OPEN increaseSalaryCursor;

        LOOP
            FETCH increaseSalaryCursor INTO castSalary;
            EXIT WHEN NOT FOUND;

            IF NOT outOfMoney THEN
                IF castSalary <= 50.00 AND totalDailyCost + 6.00 <= maxDailyCost THEN
                    totalDailyCost := totalDailyCost + 6.00;

                ELSIF castSalary <= 100 AND totalDailyCost + 8.50 <= maxDailyCost THEN
                    totalDailyCost := totalDailyCost + 8.50;

                ELSIF castSalary <= 200 AND totalDailyCost + 10.00 <= maxDailyCost THEN
                    totalDailyCost := totalDailyCost + 10.00;

                ELSE 
                    outOfMoney := TRUE;
                END IF;
            END IF;
            
            EXIT WHEN NOT outOfMoney; 
        END LOOP;

        CLOSE increaseSalaryCursor;

    RETURN totalDailyCost;

    END
$$ LANGUAGE plpgsql;