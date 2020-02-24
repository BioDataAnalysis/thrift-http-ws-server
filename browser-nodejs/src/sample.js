import thrift from 'thrift';

import TTypes from './gen-nodejs/TestThriftAPI_types';
import TestThriftAPI from './gen-nodejs/TestThriftAPI';
import assert from 'assert';

const HOST = 'localhost';
const WSPORT = 9090;

export default function browserNodeJSThriftClient() {
  const transport = thrift.TBufferedTransport;
  const protocol = thrift.TBinaryProtocol;
  console.log(`Transport used: ${transport.name}`);
  console.log(`Protocol used: ${(protocol.name)}`);

  const connection = thrift.createWSConnection(HOST, WSPORT, {
    transport : transport,
    protocol : protocol
  });
  connection.open();

  connection.on('error', function(err) {
    assert(false, err);
  });

  // Create a TestThriftAPI client with the connection
  const client = thrift.createWSClient(TestThriftAPI, connection);

/*
  // Here are a number of examples how to call thrift javascript methods...
  client.ping(function (err, response) {
    console.log('ping()');
  });

  client.ping(function (err, response) {
    console.log('ping()');
  });

  client.add(1, 1, function (err, response) {
    console.log("1+1=" + response);
  });


  const work = new TTypes.Work();
  work.op = TTypes.Operation.DIVIDE;
  work.num1 = 1;
  work.num2 = 0;

  client.calculate(1, work, function (err, message) {
    if (err) {
      console.log("InvalidOperation " + err);
    } else {
      console.log('Whoa? You know how to divide by zero?');
    }
  });

  work.op = TTypes.Operation.SUBTRACT;
  work.num1 = 15;
  work.num2 = 10;

  client.calculate(1, work, function (err, message) {
    console.log('15-10=' + message);

    client.getStruct(1, function (err, message) {
      console.log('Check log: ' + message.value);

      //close the connection once we're done
      connection.close();
    });
  });
*/

  /****************************************************************
   * Small helper methods
   ****************************************************************/
  console.log('Protocol;Transport;Data Size (Byte);Iterations;Total Duration (ms);Duration per call (ms);Calls per sec;Throughput (Byte/s)');
  function logPerformance(aName, aNumIterations, aDurationMilliSec, aDataSize) {
      const vMilliSecPerCall = aDurationMilliSec/aNumIterations;
      const vCallsPerSec = 1000.0 / vMilliSecPerCall;
      const vByteTransferredTotal = aNumIterations * aDataSize;
      const vDurationSec = aDurationMilliSec / 1000.0;
      const vBytePerSec = vByteTransferredTotal / vDurationSec;
      console.log(aName + ';' + aDataSize.toFixed(4) + ';' + aNumIterations.toFixed(4) + ';' + aDurationMilliSec.toFixed(2) + ';' + vMilliSecPerCall.toFixed(4) + ';' + vCallsPerSec.toFixed(4) + ';' + vBytePerSec.toFixed(4));
  }

  /****************************************************************
   * Benchmark helper, estimate calls per second
   ****************************************************************/
  const cAverageRuntimeSec = 4.0;
  function benchmarkSpeedEstimator(aName, aDataSizeIdx) {
      var mTimeIsUp = false;
      var mNumIterations = 1;

      function callUntilTimeout(aClient, aDataSizeIdx, aBenchStartTime) {
          aClient.fetchData(aDataSizeIdx, function(err, aResponse) {
              if (!mTimeIsUp) {
                  ++mNumIterations;
                  callUntilTimeout(aClient, aDataSizeIdx, aBenchStartTime);
              } else {
                  const vDurationMilliSec = performance.now() - aBenchStartTime;
                  const vCallsPerSec = 1000.0 * mNumIterations / vDurationMilliSec;
                  const vNumIterations = cAverageRuntimeSec * vCallsPerSec;

                  bench(aName, aClient, aDataSizeIdx, vNumIterations, vNumIterations, performance.now());
              }
          });
      }

      setTimeout(function() { mTimeIsUp = true; }, 2000);
      callUntilTimeout(client, aDataSizeIdx, performance.now());
  }

  /****************************************************************
   * Benchmark method
   ****************************************************************/
  function bench(aName, aClient, aDataSizeIdx, aIteration, aNumIterations, aBenchStartTime) {
      aClient.fetchData(aDataSizeIdx, function(err, aResponse) {
          if (aIteration > 0) {
              bench(aName, aClient, aDataSizeIdx, aIteration-1, aNumIterations, aBenchStartTime);
          } else {
              const vDurationMilliSec = performance.now() - aBenchStartTime;
              logPerformance(aName, aNumIterations, vDurationMilliSec, aResponse.length);

              runAllBenchmarks();
          }
      });
  }


  /****************************************************************
   * Orchestrator for benchmark execution
   ****************************************************************/
  var vDataSizeIdx = 0;
  function runAllBenchmarks() {
      if (vDataSizeIdx < 8) {
          benchmarkSpeedEstimator('Binary;Buffered', vDataSizeIdx);
          ++vDataSizeIdx;
      } else {
          console.log('benchmark completed.');
      }
  }
  runAllBenchmarks();

}
