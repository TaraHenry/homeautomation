<template>
    <v-container fluid align="left" justify="center">
        <!-- ROW 1 -->
        <v-row class="pd-1" style="max-width: 600px;" justify="center" align="center">
            <!-- COLUMN 1 -->
            <v-col class="col col1">
                <v-card class="pa-2 bg-background" flat height="250px" align="center">
                    <v-text-field class="mr-5" label="Start date" type="Date" density="compact" variant="solo" style="max-width: 300px;" v-model="start"></v-text-field>
                    <v-text-field class="mr-5" label="End date" type="Date" density="compact" variant="solo" style="max-width: 300px;" v-model="end"></v-text-field>
                    <v-spacer></v-spacer>
                    <v-btn class="text-caption rounded-0" text="Analyze" variant="elevated" @click="updateLineCharts();updateCards();"></v-btn>
                </v-card>
            </v-col>
            <!-- COLUMN 2 -->
            <v-col cols="4" align="center">
                <v-card title="Average" subtitle="For the selected period" width="400" height="250" variant="outlined" color="bg-surface" density="compact" rounded="lg">
                    <v-card-item align="center">
                        <span class="text-h1 text-primary font-weight-bold">
                            {{ avg.value }}
                            <span class="text-caption">Gal</span>
                        </span>
                    </v-card-item>
                </v-card>
            </v-col>
        </v-row>

        <!-- ROW 2 -->
        <v-row style="max-width: 1200px;">
            <!-- COLUMN 1 -->
            <v-col cols="12">
                <figure class="highcharts-figure">
                    <div id="container0"></div>
                </figure>
            </v-col>
            <!-- COLUMN 2 -->
            <v-col cols="12">
                <figure class="highcharts-figure">
                    <div id="container1"></div>
                </figure>
            </v-col>
        </v-row> 
    </v-container>
</template>

<script setup>
/** JAVASCRIPT HERE */

// IMPORTS
import { onBeforeUnmount, onMounted, reactive, ref } from "vue";
import { useRoute, useRouter } from "vue-router";
 
import { useAppStore } from "@/store/appStore";
import { useMqttStore } from "@/store/mqttStore"; // Import Mqtt Store
import { storeToRefs } from "pinia";

// Highcharts, Load the exporting module and Initialize exporting module.
import Highcharts from 'highcharts';
import more from 'highcharts/highcharts-more';
import Exporting from 'highcharts/modules/exporting';
Exporting(Highcharts);
more(Highcharts);

// VARIABLES
const router      = useRouter();
const route       = useRoute();  
const AppStore    = useAppStore();
const Mqtt        = useMqttStore();
const { payload, payloadTopic } = storeToRefs(Mqtt);

const start       = ref(null);
const end         = ref(null);

const waterResLChart   = ref(null);
const heightAndWaterSChart= ref(null);

var avg = reactive({ value: 0 });

// FUNCTIONS
const CreateCharts = async () => {
    // HUMIDITY CHART
    waterResLChart.value = Highcharts.chart('container0', {
        chart: { zoomType: 'x' },
        title: { text: 'Water Management Analysis', align: 'left' },
        yAxis: {
            title: { text: 'Water Reserve' , style:{color:'#000000'}},
            labels: { format: '{value} Gal' }
        },
        xAxis: {
            type: 'datetime',
            title: { text: 'Time', style:{color:'#000000'} },
        },
        tooltip: { 
            shared:true, 
            pointFormat: 'Time: {point.x} <br/> Water Reserve: {point.y} Gal'
        },
        series: [
        {
            name: 'Reserve',
            type: 'spline',
            data: [],
            turboThreshold: 0,
            color: Highcharts.getOptions().colors[0]
        }],
    });
    
    // HEIGHT AND WATER LEVEL SCATTER CHART
    heightAndWaterSChart.value = Highcharts.chart('container1', {
        chart: { zoomType: 'x' },
        title: { 
            text: 'Height and Water Level Correlation Analysis', 
            align: 'left'
        },
        yAxis: {
            title: { text: 'Height' , style:{color:'#000000'}},
            labels: { format: '{value} in' }
        },
        xAxis: {
            type: 'datetime',
            title: { text: 'Water Height', style:{color:'#000000'} },
            labels: { format: '{value} in' }
        },
        tooltip: { 
            shared:true, 
            pointFormat: 'Water Height: {point.x} in <br/> Height: {point.y} in' },
        series: [
        {
            name: 'Analysis',
            type: 'scatter',
            data: [],
            turboThreshold: 0,
            color: Highcharts.getOptions().colors[0]
        }]
    });
};

onMounted(()=>{
// THIS FUNCTION IS CALLED AFTER THIS COMPONENT HAS BEEN MOUNTED
    CreateCharts();
    Mqtt.connect(); // Connect to Broker located on the backend
    setTimeout( ()=>{
        // Subscribe to each topic
        Mqtt.subscribe("620154033");
        Mqtt.subscribe("620154033_pub");
    },3000)
});

onBeforeUnmount(()=>{
    // THIS FUNCTION IS CALLED RIGHT BEFORE THIS COMPONENT IS UNMOUNTED
    // unsubscribe from all topics
    Mqtt.unsubcribeAll();
});

const updateLineCharts = async () => {
    if (!!start.value && !!end.value) {
        // Convert output from Textfield components to 10 digit timestamps
        let startDate = new Date(start.value).getTime() / 1000;
        let endDate = new Date(end.value).getTime() / 1000;
        // Fetch data from backend
        const data = await AppStore.getRetrieveData(startDate, endDate);
        // Create arrays for each plot
        let waterReserve = [];
        let waterheight = [];

        // Iterate through data variable and transform object to format recognized by highcharts
        data.forEach((row) => {
        waterReserve.push({x: row.timestamp * 1000,y: parseFloat(row.waterReserve.toFixed(2)),});
        waterheight.push({x: row.timestamp * 1000,y: parseFloat(row.waterheight.toFixed(2)),});
        
        });
        console.log(waterReserve);
        console.log(waterheight);
        // Add data to Temperature and Heat Index chart
        waterResLChart.value.series[0].setData(waterReserve);
        heightAndWaterSChart.value.series[0].setData(waterheight);
    }
}

// const updateScatterCharts = async ()=>{
//     if(!!start.value && !!end.value){
//         // Convert output from Textfield components to 10 digit timestamps
//         let startDate = new Date(start.value).getTime() / 1000;
//         let endDate = new Date(end.value).getTime() / 1000;
        
//         // Fetch data from backend
//         const data = await AppStore.getAllInRange(startDate,endDate);

//         // Create arrays for each plot
//         let s1 = [];

//         // Iterate through data variable and transform object to format recognized by highcharts
//         data.forEach(row => {
//             s1.push({ "x": parseFloat(row.waterHeight.toFixed(2)) , "y": parseFloat(row.height.toFixed(2)) });
//         });
//         tempHiSChart.value.series[0].setData(s1);
//     }
// }

const updateCards = async () => {
    // Retrieve Min, Max, Avg, Spread/Range
    if(!!start.value && !!end.value){
        // 1. Convert start and end dates collected fron TextFields to 10 digit timestamps
        let startDate = new Date(start.value).getTime() / 1000;
        let endDate = new Date(end.value).getTime() / 1000;
        // 2. Fetch data from backend by calling the API functions
        const average = await AppStore.getCalculateAvg(startDate,endDate);
        console.log(average)
        avg.value = average[0].average.toFixed(1)*10;
    }
}

</script>


<style scoped>
/** CSS STYLE HERE */
Figure {
border: 2px solid black;
}

</style>