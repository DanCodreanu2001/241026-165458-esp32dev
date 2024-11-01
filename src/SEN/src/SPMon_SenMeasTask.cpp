/******************************************************************************************************
 * Project name: Solar Panel Monitoring
 * Author: Codreanu Dan
 * File name: SPMon_SenMeasTask.cpp
 * Descr: Contains the functions for SPMon_SenMeasTask
 *
 *
 *
 *
 *
 * Version: 1.0, 28.10.2024
 * *****************************************************************************************************/

/******************************************************************************************************
 * IMPORT AREA
 * *****************************************************************************************************/
#include "SPMon_DataTypes.h"
#include "SPMon_SenMeasTask.h"
#include "SPMon_lm35_lib.h"
#include <vector>

/******************************************************************************************************
 * FILE LOCAL VARIABLES
 * *****************************************************************************************************/
bool SPMon_SensorMeasurementTask::taskCreated = FALSE;

/******************************************************************************************************
 * Function name: SPMon_SenMeasTask_MainFunc
 * Descr: Main function for the SPMon_SenMeasTask_MainFunc protoype
 * Params: none
 *
 * *****************************************************************************************************/
void SPMon_SenMeasTask_MainFunc(void *parameter)
{
  for (;;)
  {
    Serial.println(F("[SEN_MEAS_TASK_MAIN_FUNC/called_once_per_second]"));
    /* Execute the state machine */
    sensor_measurement.SPMon_SenMeasTask_ExecuteStateMachine((TaskStateMng *)parameter);
    vTaskDelay(SEN_MEAS_TASK_PERIOD / portTICK_PERIOD_MS);
  }
}

/******************************************************************************************************
 * Function name: SPMon_SenMeasTask_ExecuteStateMachine
 * Descr: Function that executes the state machine for the sensor measurement task
 * Params: TaskStateMng *taskState - pointer to the task state management structure
 * Returns: none
 *
 * ******************************************************************************************************/
void SPMon_SensorMeasurementTask::SPMon_SenMeasTask_ExecuteStateMachine(TaskStateMng *taskState)
{
  Serial.println(F("[SPMon_SenMeasTask_ExecuteStateMachine]"));
  vTaskDelay(1000 / portTICK_PERIOD_MS);

  /* Create a vector of sensor measurement interfaces */
  std::vector<SPMon_SensorMeasurementTask_Interface *> sensorMeasurementInterface;
  /* Add the sensor measurement interfaces to the vector */
  sensorMeasurementInterface.push_back(new SPMon_SensorMeasurementTask_LM35());

  switch (taskState->SenMeasTaskState)
  {
    /* Execute the state machine */
    case SENS_MEAS_STATE_OFF:
      Serial.println(F("[STATE] SENS_MEAS_STATE_OFF"));
      taskState->SenMeasTaskState = SENS_MEAS_STATE_MEAS; // Transition to MEAS state
      break;
    case SENS_MEAS_STATE_MEAS:
      Serial.println(F("[STATE] SENS_MEAS_STATE_MEAS"));
      /* Get raw data from the sensors */
      for (SPMon_SensorMeasurementTask_Interface *object : sensorMeasurementInterface)
      {
        if (object != nullptr)
        {
          /* Call the function */
          SensorRawValues localRawValues = {0}; // Initialize to zero
          object->SPMon_SenMeasTask_GetRawData(&localRawValues);
          Serial.print(F("[RAW_DATA]: "));
          Serial.println(localRawValues.RawAdc_TempVal_LM35); // Print raw data
          /* Delete the object */
          delete object;
        }
        else
        {
          Serial.println(F("[ERROR] Null pointer detected in SENS_MEAS_STATE_MEAS"));
        }
      }
      taskState->SenMeasTaskState = SENS_MEAS_STATE_CONV; // Transition to CONV state
      break;
    case SENS_MEAS_STATE_CONV:
      Serial.println(F("[STATE] SENS_MEAS_STATE_CONV"));
      /* Convert raw data from the sensors */
      for (SPMon_SensorMeasurementTask_Interface *object : sensorMeasurementInterface)
      {
        if (object != nullptr)
        {
          /* Call the function */
          SensorRawValues localRawValues = {0}; // Initialize to zero
          SensorConvertedValues localConvertedValues = {0}; // Initialize to zero
          SensorErrorMonitoring localSensorError = {SENS_TEMP_LM_35, SEN_ERROR_NO_ERROR, false, 0}; // Initialize to default values
          object->SPMon_SenMeasTask_ConvertData(&localRawValues, &localConvertedValues, &localSensorError);
          Serial.print(F("[CONVERTED_DATA]: "));
          Serial.println(localConvertedValues.ConValTempLM35); // Print converted data
          /* Delete the object */
          delete object;
        }
        else
        {
          Serial.println(F("[ERROR] Null pointer detected in SENS_MEAS_STATE_CONV"));
        }
      }
      taskState->SenMeasTaskState = SENS_MEAS_STATE_MEAS; // Transition back to MEAS state
      break;
    case SENS_MEAS_STATE_ERROR:
      Serial.println(F("[STATE] SENS_MEAS_STATE_ERROR"));
      break;
    default:
      Serial.println(F("[ERROR] Unknown state"));
      break;
  }
}

/******************************************************************************************************
 * Function name: SPMon_SenMeasTask_CreateSenMeasTask
 * Descr: Function that creates the SPMon_SenMeasTask
 * Params:
 * Rs: bool - TRUE if the task is created, FALSE if the task is already created
 *
 * *****************************************************************************************************/
bool SPMon_SensorMeasurementTask::SPMon_SenMeasTask_CreateSenMeasTaskTask()
{

  /* Check if the task is created */
  if (!taskCreated)
  {
    /* Create task */
    if (xTaskCreatePinnedToCore(
            /* Task main function */
            SPMon_SenMeasTask_MainFunc,
            /* Task name */
            "SPMon_SenMeasTask",
            /* Stack size (bytes) */
            STACK_SIZE_BYTES,
            /* Parameter */
            new TaskStateMng(), // Pass a new TaskStateMng instance
            /* Task priority */
            SPMon_SenMeasTask_prio,
            /* Task handler */
            &SPMon_SenMeasTask_hdl,
            /* Task running on core (1 or 0) */
            1) == pdPASS)
    {
      Serial.println(F("[SEN_MEAS_TASK_CREATED]"));
    }
    else
    {
      Serial.println(F("[ERROR] Failed to create SEN_MEAS_TASK"));
    }
    taskCreated = TRUE;
    return TRUE;
  }
  else
  {
    return FALSE;
  }
}

/******************************************************************************************************
 * LM 35 SENSOR FUNCTIONS
 *
 * *****************************************************************************************************/

/******************************************************************************************************
 * Constructor
 * ******************************************************************************************************/
SPMon_SensorMeasurementTask_LM35::SPMon_SensorMeasurementTask_LM35()
{
}

/******************************************************************************************************
 * Function name: SPMon_SenMeasTask_GetRawDataLM35
 * Descr: Function that gets the raw data from the LM35 sensor, overrides the virtual function from the
 *        SPMon_SensorMeasurementTask_Interface class
 * Params: SensorRawValues * rawValues - pointer to the raw values structure
 *
 * ******************************************************************************************************/
void SPMon_SensorMeasurementTask_LM35::SPMon_SenMeasTask_GetRawData(SensorRawValues * rawValues)
{
  if (rawValues != nullptr)
  {
    /* Get raw data from the LM35 sensor */
    Serial.print(F("[LM35_GET_RAW_DATA]: "));
    /* Get raw data from the LM35 sensor */
    lm35.LM35_GetRawData(rawValues);
    Serial.println(rawValues->RawAdc_TempVal_LM35); // Print raw data
  }
  else
  {
    Serial.println(F("[ERROR] Null pointer passed to SPMon_SenMeasTask_GetRawData"));
  }
}

/******************************************************************************************************
 * Function name: SPMon_SenMeasTask_ConvertDataLM35
 * Descr: Function that converts the raw data from the LM35 sensor, overrides the virtual function from the
 *        SPMon_SensorMeasurementTask_Interface class
 * Params: SensorRawValues * rawValues - pointer to the raw values structure
 *         SensorConvertedValues * convertedValues - pointer to the converted values structure
 *         SensorErrorMonitoring * sensorError - pointer to the sensor error structure
 *
 * *****************************************************************************************************/
void SPMon_SensorMeasurementTask_LM35::SPMon_SenMeasTask_ConvertData(SensorRawValues * rawValues, SensorConvertedValues * convertedValues, SensorErrorMonitoring * sensorError)
{
  if (rawValues != nullptr && convertedValues != nullptr && sensorError != nullptr)
  {
    /* Convert raw data from the LM35 sensor */
    Serial.print(F("[LM35_CONVERT_DATA]: "));
    lm35.LM35_GetTemp(rawValues, convertedValues, sensorError);
    Serial.println(convertedValues->ConValTempLM35); // Print converted data
  }
  else
  {
    Serial.println(F("[ERROR] Null pointer passed to SPMon_SenMeasTask_ConvertData"));
  }
}

/******************************************************************************************************
 * Function name: SPMon_SenMeasTask_ErrorMonitorLM35
 * Descr: Function that monitors the errors from the LM35 sensor, overrides the virtual function from the
 *       SPMon_SensorMeasurementTask_Interface class
 * Params: SensorErrorMonitoring * sensorError - pointer to the sensor error structure
 *
 * ******************************************************************************************************/
void SPMon_SensorMeasurementTask_LM35::SPMon_SenMeasTask_ErrorMonitor()
{
}