/*
 # Arquivo:

 UnicViewAD.h

 # Projeto:

 UnicViewAD.lcm - Biblioteca Proculus em C

 # Descri??o:

 Esta biblioteca permite que um microcontrolador se comunique facilmente com o Displays
 Inteligentes Proculus, oferecendo um controle mais intuitivo e pr?tico de todas
 as suas fun??es.

 Ela elimina a necessidade de um completo entendimento sobre todos os protocolos de
 comunica??o e funcionamento interno do Display, abstraindo esses protocolos para
 pequenas fun??es que garantem facilidade e rapidez de uso.

 # Autores:

 * Andreo de Freitas
 * Rafael Moura (revis?o)

 # Vers?o:

 1.0

 # Licen?a:

 Copyright (c) 2020 Proculus Technologies Limited and Victor Vision LTDA

 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.

 */
	
#include "unicviewad_c.h"

#define LCM_NULL NULL
#define LCM_RX_PACKAGE_NULL NULL

static enum lcm_crc_activated
{
    LCM_CRC_FALSE, LCM_CRC_TRUE
};

static enum lcm_crc_verification
{
    LCM_CRC_NOT_OK, LCM_CRC_OK
};

typedef struct lcm_node
{
    lcm_rx_package_t* p_rx_package;
    struct lcm_node* next;

} lcm_node_t;

typedef struct
{
    lcm_node_t* head;
    lcm_node_t* tail;

} lcm_queue_t;

static uint8_t* g_p_lcm_rx_buffer;
static uint8_t* g_p_lcm_data_buffer;
static uint8_t g_lcm_rx_complete_received = 0;

static uint16_t g_lcm_audio_id = 0;
static uint8_t g_lcm_incoming_rx_bytes_received = 0;
static uint8_t g_lcm_incoming_rx_remaining_bytes = 0;

static uint8_t g_lcm_register_activate_command = 0x5A;
static uint8_t g_lcm_write_register_command = LCM_WRITE_REGISTER_COMMAND;
static uint8_t g_lcm_read_register_command = LCM_READ_REGISTER_COMMAND;
static uint8_t g_lcm_write_vp_command = LCM_WRITE_VP_COMMAND;
static uint8_t g_lcm_read_vp_command = LCM_READ_VP_COMMAND;

static uint16_t g_lcm_null = LCM_EMPTY_RESPONSE;

static uint8_t g_lcm_string_terminator = LCM_STRING_TERMINATOR;

static lcm_rtc_datetime_t g_lcm_rtc;

static uint32_t g_lcm_read_timeout = 0x7FFFF0;
static uint8_t g_lcm_use_delay = 1u;

static uint32_t g_lcm_garbage_collector_max_ticks = 500000;

static uint8_t g_lcm_header_high_byte = 0x5A;
static uint8_t g_lcm_header_low_byte = 0xA5;
static uint8_t g_lcm_use_crc = LCM_CRC_FALSE;

extern uint8_t lcm_write_bytes(uint8_t* data, uint8_t length); ////////// INTERFACE PUBLICA

extern uint16_t lcm_read_byte();  ////////// INTERFACE PUBLICA

void lcm_update_data();

/******************************************************************************************************************************
 * RxPackage
 */

static lcm_queue_t* lcm_rx_package_queue;
static lcm_node_t* lcm_node_null;
static uint8_t* lcm_data_buffer_null;

static void lcm_rx_package_new_package(lcm_rx_package_t* lcm_rx_package,
    uint8_t length)
{
    lcm_rx_package->p_data_buffer = (uint8_t*)malloc(length * sizeof(uint8_t));
}

static void lcm_rx_package_set_data(lcm_rx_package_t* lcm_rx_package,
    uint8_t* data)
{
    uint8_t length = lcm_rx_package->rx_length;

    if (lcm_rx_package->rx_type == g_lcm_read_vp_command)
    {
        length *= 2;
    }

    memcpy(lcm_rx_package->p_data_buffer, data, length);
}

static void lcm_rx_package_set_data_index(lcm_rx_package_t* lcm_rx_package,
    uint8_t value)
{
    lcm_rx_package->data_buffer_index = value;
}

static void lcm_rx_package_set_rx_type(lcm_rx_package_t* lcm_rx_package,
    uint8_t value)
{
    lcm_rx_package->rx_type = value;
}

static void lcm_rx_package_set_rx_address(lcm_rx_package_t* lcm_rx_package,
    uint16_t value)
{
    lcm_rx_package->rx_address = value;
}

static void lcm_rx_package_set_rx_length(lcm_rx_package_t* lcm_rx_package,
    uint8_t value)
{
    lcm_rx_package->rx_length = value;
}

static uint8_t lcm_rx_package_get_data(lcm_rx_package_t* lcm_rx_package,
    uint8_t** buffer)
{
    *buffer = lcm_rx_package->p_data_buffer;

    return 1;
}

static uint8_t lcm_rx_package_get_data_index(lcm_rx_package_t* lcm_rx_package)
{
    return lcm_rx_package->data_buffer_index;
}

static uint8_t lcm_rx_package_get_rx_type(lcm_rx_package_t* lcm_rx_package)
{
    return lcm_rx_package->rx_type;
}

static uint16_t lcm_rx_package_get_rx_address(lcm_rx_package_t* lcm_rx_package)
{
    return lcm_rx_package->rx_address;
}

static uint8_t lcm_rx_package_get_rx_length(lcm_rx_package_t* lcm_rx_package)
{
    return lcm_rx_package->rx_length;
}

/******************************************************************************************************************************
 * Queue
 */

static void lcm_queue_new_queue()
{
    lcm_node_null = (lcm_node_t*)malloc(sizeof(lcm_node_t));

    lcm_rx_package_queue = (lcm_queue_t*)malloc(sizeof(lcm_queue_t));

    lcm_data_buffer_null = (uint8_t*)malloc(sizeof(uint8_t));
	
    lcm_rx_package_queue->head = lcm_node_null;
    lcm_rx_package_queue->head->p_rx_package = LCM_RX_PACKAGE_NULL;
    lcm_rx_package_queue->head->next = lcm_node_null;
    lcm_rx_package_queue->tail = lcm_rx_package_queue->head;

}

static void lcm_queue_delete_queue( )
{
    for (lcm_node_t* lcm_node = lcm_rx_package_queue->head; lcm_node != LCM_NULL;
        lcm_node = lcm_rx_package_queue->head)
    {
        lcm_rx_package_queue->head = lcm_node->next;
        free(lcm_node);
    }
}

static uint8_t lcm_queue_enqueue( lcm_rx_package_t* lcm_rx_package)
{
    lcm_node_t* lcm_node = (lcm_node_t*)malloc(sizeof(lcm_node_t));

    if (LCM_NULL == lcm_node)
    {
        free(lcm_rx_package);
        return 0;
    }

    lcm_node->next = lcm_node_null;

    lcm_node->p_rx_package = lcm_rx_package;

    lcm_node->p_rx_package->garbage_collector_counter = 0;

    if (lcm_node_null == lcm_rx_package_queue->head)
    {
        lcm_rx_package_queue->head = lcm_node;
        lcm_rx_package_queue->tail = lcm_node;
    }
    else
    {
        lcm_rx_package_queue->tail->next = lcm_node;
        lcm_rx_package_queue->tail = lcm_node;
    }

    return 1;
}

static void lcm_queue_removing( lcm_node_t* lcm_node_previous_node, lcm_node_t* lcm_node_current_node)
{
    if (lcm_node_current_node == lcm_rx_package_queue->head)
    {
        lcm_rx_package_queue->head = lcm_node_current_node->next;
        if (lcm_rx_package_queue->head == lcm_node_null)
        {
            lcm_rx_package_queue->tail = lcm_node_null;
        }
        free(lcm_node_current_node->p_rx_package->p_data_buffer);
        free(lcm_node_current_node->p_rx_package);
        free(lcm_node_current_node);
    }
    else
    {
        lcm_node_previous_node->next = lcm_node_current_node->next;
        if (lcm_node_current_node == lcm_rx_package_queue->tail)
        {
            lcm_rx_package_queue->tail = lcm_node_previous_node;
        }
        free(lcm_node_current_node->p_rx_package->p_data_buffer);
        free(lcm_node_current_node->p_rx_package);
        free(lcm_node_current_node);
    }
}

static lcm_rx_package_t* lcm_queue_search(uint8_t type,
    uint16_t address)
{
    lcm_rx_package_t* lcm_rx_package = LCM_RX_PACKAGE_NULL;

    if (lcm_node_null != lcm_rx_package_queue->head)
    {
        lcm_node_t* lcm_node_previous_node = lcm_node_null;
        lcm_node_t* lcm_node_current_node = lcm_rx_package_queue->head;
        while (lcm_node_null != lcm_node_current_node)
        {
            if ((type == lcm_node_current_node->p_rx_package->rx_type)
                && (address
                    == lcm_node_current_node->p_rx_package->rx_address))
            {
                if (lcm_node_current_node->p_rx_package->data_buffer_index
                    >= lcm_node_current_node->p_rx_package->rx_length)
                {
                    lcm_queue_removing(lcm_node_previous_node,
                        lcm_node_current_node);

                }
                else
                {
                    lcm_rx_package = lcm_node_current_node->p_rx_package;
                }
                lcm_rx_package->garbage_collector_counter = 0;
                return lcm_rx_package;
            }
            else
            {
                if ((lcm_node_current_node->p_rx_package->garbage_collector_counter
                    == g_lcm_garbage_collector_max_ticks) && (g_lcm_garbage_collector_max_ticks > 0))
                {
                    ++lcm_node_current_node->p_rx_package->garbage_collector_counter;
                    lcm_queue_removing(lcm_node_previous_node,
                        lcm_node_current_node);
                }
                else
                {
                    ++lcm_node_current_node->p_rx_package->garbage_collector_counter;

                    if (lcm_node_current_node->p_rx_package->data_buffer_index
                        >= lcm_node_current_node->p_rx_package->rx_length)
                    {
                        lcm_queue_removing(lcm_node_previous_node,
                            lcm_node_current_node);
                    }
                    else
                    {
                        lcm_node_previous_node = lcm_node_current_node;
                    }
                }
            }
            lcm_node_current_node = lcm_node_current_node->next;
        }
    }
    return lcm_rx_package;
}

static uint8_t lcm_queue_dequeue( uint8_t type,
    uint16_t vp)
{
    lcm_node_t* lcm_node_previous_node = LCM_NULL;

    for (lcm_node_t* lcm_node_current_node = lcm_rx_package_queue->head;
        lcm_node_current_node != LCM_NULL; lcm_node_current_node =
		lcm_node_current_node->next)
    {
        if ((type
            == lcm_rx_package_get_rx_type(
                lcm_node_current_node->p_rx_package))
            && (vp
                == lcm_rx_package_get_rx_address(
                    lcm_node_current_node->p_rx_package)))
        {
            lcm_queue_removing( lcm_node_previous_node,
                lcm_node_current_node);

            lcm_node_current_node = LCM_NULL;
            return 1;
        }
        else
        {
            lcm_node_previous_node = lcm_node_current_node;
        }
    }
    return 0;
}

static lcm_rx_package_t* lcm_queue_first( )
{
    if (LCM_RX_PACKAGE_NULL != lcm_rx_package_queue->head)
    {
        return lcm_rx_package_queue->head->p_rx_package;
    }
    else
    {
        return LCM_RX_PACKAGE_NULL;
    }
}

static void lcm_queue_garbage_collector()
{
    lcm_queue_search( 1, 0xFFFE);
}

static uint8_t lcm_queue_is_empty( )
{
    return (lcm_rx_package_queue->head == lcm_node_null);
}

/***************************************************************************
 *
 */

static lcm_rx_package_t* lcm_wait_response(uint8_t type, uint16_t address)
{
    uint32_t wait_ticks = 0;
    lcm_rx_package_t* lcm_rx_package_temp = LCM_RX_PACKAGE_NULL;
    while (wait_ticks < g_lcm_read_timeout)
    {
        lcm_update_data();

        if (LCM_RX_PACKAGE_NULL != lcm_rx_package_temp)
        {
            return lcm_rx_package_temp;
        }
        else
        {
            lcm_rx_package_temp = lcm_queue_search( type,
                address);
        }
        wait_ticks++;
    }
    return lcm_rx_package_temp;
}

/***************************************************************************/
/* Fun??es de tratamento de dado */

uint8_t lcm_int_to_bcd(uint8_t value)
{
    return ((value % 10) | ((value / 10) << 4));
}

uint8_t lcm_bcd_to_int(uint8_t value)
{
    return (value - (6 * (value >> 4)));
}

static uint8_t lcm_high_byte_of_word(uint16_t input)
{
    return (input >> 8);
}

static uint8_t lcm_low_byte_of_word(uint16_t input)
{
    return input;
}

static uint16_t lcm_make16(uint8_t input_high, uint8_t input_low)
{
    uint16_t output = input_high;

    output = output << 8;

    output = output | input_low;

    return ((uint16_t)input_high << 8) | input_low;
}

static uint16_t lcm_crc_calculator(uint8_t* buffer, uint8_t length)
{
    uint16_t crc = 0xFFFF;

    for (uint8_t pos = 0; pos < length; pos++)
    {
        // XOR byte into least sig. byte of crc
        crc ^= (uint16_t) * (buffer + pos);
        for (uint8_t i = 8; 0 != i; i--) // Loop over each bit
        {
            if (0 != (0x0001 & crc)) // If the LSB is set
            {
                crc >>= 1; // Shift right and XOR 0xA001
                crc ^= 0xA001;
            }
            else // Else LSB is not set
            {
                crc >>= 1; // Just shift right

            }
        }
    }
    // Swap high and low bytes
    crc = ((crc & 0xFF00) >> 8) | ((crc & 0x00FF) << 8);
    return crc;
}

/***************************************************************************/
/* Fun??es de Escrita em VP */
/*
uint8_t lcm_write_bytes_crc(uint8_t* buffer, uint8_t length)
{
    *(buffer + 2) += 2;

    uint16_t crc16 = lcm_crc_calculator(&*(buffer + 3), length - 3);

    uint8_t crc[2];
    crc[0] = lcm_high_byte_of_word(crc16);
    crc[1] = lcm_low_byte_of_word(crc16);

    uint8_t tx_length = length + 2;

    uint8_t* tx_buffer = (uint8_t*)malloc(tx_length);

    memcpy(tx_buffer, buffer, length);

    *(tx_buffer + length) = crc[0];
    *(tx_buffer + length + 1) = crc[1];

    lcm_write_bytes(tx_buffer, tx_length);

    free(tx_buffer);

    return 0;
}
*/

#define NUMBER_OF_BUFFER_OUT_BYTES 3
uint8_t* buffer_out_bytes[NUMBER_OF_BUFFER_OUT_BYTES];
uint8_t index_of_buffer_out_bytes = 0;

uint8_t lcm_write_out_bytes(uint8_t* data, uint8_t length)
{

    if (buffer_out_bytes[index_of_buffer_out_bytes] != LCM_NULL)
    {
        free(buffer_out_bytes[index_of_buffer_out_bytes]);
    }

    uint8_t return_status = 0;

    if (LCM_CRC_TRUE == g_lcm_use_crc)
    {
        *(data + 2) += 2;

        uint16_t crc16 = lcm_crc_calculator(&*(data + 3), length - 3);

        uint8_t tx_length = length + 2;

        buffer_out_bytes[index_of_buffer_out_bytes] = (uint8_t*)malloc(tx_length);

        memcpy(buffer_out_bytes[index_of_buffer_out_bytes], data, length);

        *(buffer_out_bytes[index_of_buffer_out_bytes] + length) = lcm_high_byte_of_word(crc16);
        *(buffer_out_bytes[index_of_buffer_out_bytes] + length + 1) = lcm_low_byte_of_word(crc16);

        length = tx_length;

        free(data);
    }
    else
    {
        buffer_out_bytes[index_of_buffer_out_bytes] = data;
    }

    return_status = lcm_write_bytes(buffer_out_bytes[index_of_buffer_out_bytes], length);
	
    index_of_buffer_out_bytes++;

    if (NUMBER_OF_BUFFER_OUT_BYTES == index_of_buffer_out_bytes)
    {
        index_of_buffer_out_bytes = 0;
    }

    return return_status;
}


/*
uint8_t lcm_write_out_bytes(uint8_t* data, uint8_t length)
{

    buffer_out_bytes[index_of_buffer_out_bytes] = (uint8_t*)realloc(buffer_out_bytes[index_of_buffer_out_bytes],
        length * sizeof(uint8_t));

    memcpy(buffer_out_bytes[index_of_buffer_out_bytes], data, length);
	
    uint8_t return_status = 0;
	
    if (LCM_CRC_TRUE == g_lcm_use_crc)
    {
        return_status = lcm_write_bytes_crc(buffer_out_bytes[index_of_buffer_out_bytes], length);
    }
    else
    {
        return_status = lcm_write_bytes(buffer_out_bytes[index_of_buffer_out_bytes], length);
    }

    index_of_buffer_out_bytes++;
	
	if (NUMBER_OF_BUFFER_OUT_BYTES == index_of_buffer_out_bytes)
	{
        index_of_buffer_out_bytes = 0;
	}	
	
    return return_status;
}
*/


uint8_t lcm_write_vp(uint16_t vp, uint16_t value)
{
    return lcm_write_multiple_vps(vp, &value, 1u);
}

uint8_t lcm_write_multiple_vps(uint16_t vp, uint16_t* value, uint8_t length)
{
    uint8_t byte_count = (length << 1) + 3;
    uint8_t total_length = byte_count + 3;
    uint8_t pre_buffer_length = 6;

    uint8_t* buffer = (uint8_t*)malloc(total_length);

    *(buffer) = g_lcm_header_high_byte;
    *(buffer + 1) = g_lcm_header_low_byte;
    *(buffer + 2) = byte_count;
    *(buffer + 3) = g_lcm_write_vp_command;
    *(buffer + 4) = lcm_high_byte_of_word(vp);
    *(buffer + 5) = lcm_low_byte_of_word(vp);

    for (uint8_t i = 0; i < length; i++)
    {
        *(buffer + (i << 1) + pre_buffer_length) = lcm_high_byte_of_word(
            *(value + i));
        *(buffer + ((i << 1) + 1) + pre_buffer_length) = lcm_low_byte_of_word(
            *(value + i));
    }

    lcm_update_data();

    uint8_t return_status = 0;
	
    return_status = lcm_write_out_bytes(buffer, total_length);
/**********************************************************************************************************
    free(buffer);*/

    return return_status;
}

/***************************************************************************/
/* Fun??es de Escrita em Registrador */

uint8_t lcm_write_register(uint8_t reg, uint8_t value)
{
    return lcm_write_multiple_registers(reg, &value, 1u);
}

uint8_t lcm_write_multiple_registers(uint8_t reg, uint8_t* value,
    uint8_t length)
{
    uint8_t byte_count = length + 2;
    uint8_t total_length = byte_count + 3;
    uint8_t pre_buffer_length = 5;

    uint8_t* buffer = (uint8_t*)malloc(total_length);

    *(buffer) = g_lcm_header_high_byte;
    *(buffer + 1) = g_lcm_header_low_byte;
    *(buffer + 2) = byte_count;
    *(buffer + 3) = g_lcm_write_register_command;
    *(buffer + 4) = reg;

    memcpy(buffer + pre_buffer_length, value, length);

    lcm_update_data();

    uint8_t return_status = 0;

    return_status = lcm_write_out_bytes(buffer, total_length);

    /**********************************************************************************************************
    free(buffer);*/

    return return_status;
}

/***************************************************************************/
/* Fun??es de leitura em VPs */

uint16_t lcm_read_vp(uint16_t vp)
{/*
    lcm_request_vp(vp, 1u);

    lcm_rx_package_t* lcm_rx_package_temp = lcm_wait_response(
        g_lcm_read_vp_command, vp);

    if (LCM_RX_PACKAGE_NULL == lcm_rx_package_temp)
    {
        return 0xFFFF;
    }
    else
    {
        return lcm_make16(*(lcm_rx_package_temp->p_data_buffer),
            *(lcm_rx_package_temp->p_data_buffer + 1));
    }
    */

    uint16_t buffer = 0;

    lcm_read_multiple_vps(vp, &buffer, 1u);

    return buffer;
}

uint8_t lcm_read_multiple_vps(uint16_t vp, uint16_t* value, uint8_t length)
{
    lcm_request_vp(vp, length);

    lcm_rx_package_t* lcm_rx_package_temp = lcm_wait_response(
        g_lcm_read_vp_command, vp);

    if (LCM_RX_PACKAGE_NULL == lcm_rx_package_temp)
    {
        return 0;
    }

    uint8_t i = 0;
    
    for (; i < length; i++)
    {
        *(value + i) = lcm_make16(
            *(lcm_rx_package_temp->p_data_buffer + (i << 1)),
            *(lcm_rx_package_temp->p_data_buffer + (i << 1) + 1));
    }

    lcm_rx_package_temp->rx_length = 0;

    lcm_queue_garbage_collector();

    return length;
}

uint8_t lcm_request_vp(uint16_t vp, uint8_t length)
{

    uint8_t* buffer = (uint8_t*)malloc(7u);

    *(buffer) = g_lcm_header_high_byte;
    *(buffer + 1) = g_lcm_header_low_byte;
    *(buffer + 2) = 0x04;
    *(buffer + 3) = g_lcm_read_vp_command;
    *(buffer + 4) = lcm_high_byte_of_word(vp);
    *(buffer + 5) = lcm_low_byte_of_word(vp);
    *(buffer + 6) = length;

    lcm_update_data();
	
    lcm_write_out_bytes(buffer, 7U);

    /**********************************************************************************************************
    free(buffer);*/
    return 1;
}

/***************************************************************************/
/* Fun??es de leitura em Registradores */

uint8_t lcm_read_register(uint8_t reg)
{
    uint8_t buffer = 0;

    lcm_read_multiple_registers(reg, &buffer, 1u);

    return buffer;
}

uint8_t lcm_read_multiple_registers(uint8_t reg, uint8_t* value, uint8_t length)
{
    lcm_request_register(reg, length);

    lcm_rx_package_t* lcm_rx_package_temp = lcm_wait_response(
        g_lcm_read_register_command, reg);

    if (LCM_RX_PACKAGE_NULL == lcm_rx_package_temp)
    {
        return 0;
    }

    memcpy(value, lcm_rx_package_temp->p_data_buffer, length);

    lcm_rx_package_temp->rx_length = 0;

    lcm_queue_garbage_collector();

    return length;
}

uint8_t lcm_request_register(uint8_t reg, uint8_t length)
{
    lcm_update_data();

    uint8_t* buffer = (uint8_t*)malloc(6u);

    *(buffer + 0) = g_lcm_header_high_byte;
    *(buffer + 1) = g_lcm_header_low_byte;
    *(buffer + 2) = 0x03;
    *(buffer + 3) = g_lcm_read_register_command;
    *(buffer + 4) = reg;
    *(buffer + 5) = length;

    lcm_write_out_bytes(buffer, 6U);

    /**********************************************************************************************************
    free(buffer);*/

    return 1;
}

/***************************************************************************/
/* Fun??es de controle de recebimento */

static uint8_t lcm_verify_crc()
{
    uint8_t length = 0;

    if (g_lcm_read_vp_command == *(g_p_lcm_rx_buffer + 3))
    {
        length = (*(g_p_lcm_rx_buffer + 6) << 1) + 4;
    }
    else
    {
        length = *(g_p_lcm_rx_buffer + 5) + 3;
    }

    uint16_t crc16 = 0;

    crc16 = lcm_crc_calculator(g_p_lcm_rx_buffer + 3, length);

    uint8_t crc[2];

    crc[0] = lcm_high_byte_of_word(crc16);
    crc[1] = lcm_low_byte_of_word(crc16);

    uint8_t index = length + 3;

    if (*(g_p_lcm_rx_buffer + index) != crc[0])
    {
        return LCM_CRC_NOT_OK;
    }

    ++index;
    if (*(g_p_lcm_rx_buffer + index) != crc[1])
    {
        return LCM_CRC_NOT_OK;
    }

    return LCM_CRC_OK;
}

static uint8_t lcm_update_data_buffer()
{
    if (1 == g_lcm_rx_complete_received)
    {
        if (LCM_CRC_TRUE == g_lcm_use_crc)
        {
            if (LCM_CRC_NOT_OK == lcm_verify_crc())
            {
                return 0;
            }
        }

        lcm_rx_package_t* lcm_rx_package = (lcm_rx_package_t*)malloc(
            sizeof(lcm_rx_package_t));

        if (LCM_NULL != lcm_rx_package)
        {

            lcm_rx_package->rx_type = *(g_p_lcm_rx_buffer + 3);

            lcm_rx_package->data_buffer_index = 0;

            uint8_t length = 0;

            switch (lcm_rx_package->rx_type)
            {
            case LCM_READ_REGISTER_COMMAND:
            {
                lcm_rx_package->rx_address = *(g_p_lcm_rx_buffer + 4);
                lcm_rx_package->rx_length = *(g_p_lcm_rx_buffer + 5);
                g_p_lcm_data_buffer = g_p_lcm_rx_buffer + 6;
                length = lcm_rx_package->rx_length;
            }
            break;

            case LCM_READ_VP_COMMAND:
            {
                lcm_rx_package->rx_address = lcm_make16(
                    *(g_p_lcm_rx_buffer + 4), *(g_p_lcm_rx_buffer + 5));
                lcm_rx_package->rx_length = *(g_p_lcm_rx_buffer + 6);

                g_p_lcm_data_buffer = g_p_lcm_rx_buffer + 7;
                length = lcm_rx_package->rx_length << 1;
            }
            break;

            default:
                break;
            }

            lcm_rx_package->p_data_buffer = (uint8_t*)malloc(
                length * sizeof(uint8_t));

            memcpy(lcm_rx_package->p_data_buffer, g_p_lcm_data_buffer, length);

            g_lcm_rx_complete_received = 0;
        	
            return lcm_queue_enqueue(lcm_rx_package);        	
        }
    }
    return 0;
}

static uint8_t lcm_parse_rx_buffer()
{
    uint16_t parse_byte = lcm_read_byte();

    if (g_lcm_null != parse_byte)
    {
        switch (g_lcm_incoming_rx_bytes_received)
        {
        case 0:
            if (parse_byte == g_lcm_header_high_byte)
            {
                ++g_lcm_incoming_rx_bytes_received;
            }
            break;
        case 1:
            if (parse_byte == g_lcm_header_low_byte)
            {
                ++g_lcm_incoming_rx_bytes_received;
            }
            else
            {
                g_lcm_incoming_rx_bytes_received = 0;
            }
            break;
        case 2:
            g_lcm_incoming_rx_remaining_bytes = parse_byte + 3;

            g_p_lcm_rx_buffer = (uint8_t*)realloc(g_p_lcm_rx_buffer,
                g_lcm_incoming_rx_remaining_bytes * sizeof(uint8_t));

            ++g_lcm_incoming_rx_bytes_received;
            break;
        default:
        {
            *(g_p_lcm_rx_buffer + g_lcm_incoming_rx_bytes_received) =
                parse_byte;

            ++g_lcm_incoming_rx_bytes_received;

            if (g_lcm_incoming_rx_remaining_bytes
                == g_lcm_incoming_rx_bytes_received)
            {

                g_lcm_rx_complete_received = 1;
                g_lcm_incoming_rx_bytes_received = 0;
                return 1;
            }
        }
        break;
        }
    }
    return 0;
}

void lcm_update_data()
{
	lcm_parse_rx_buffer();
	lcm_update_data_buffer();
}

lcm_rx_package_t* g_lcm_rx_package = LCM_RX_PACKAGE_NULL;

uint8_t lcm_available()
{
    lcm_update_data();
	
    g_lcm_rx_package = lcm_queue_first();

    return (LCM_RX_PACKAGE_NULL != lcm_queue_first());
}

uint8_t lcm_get_data_array(uint16_t* buffer, uint8_t length)
{
    if (LCM_RX_PACKAGE_NULL != g_lcm_rx_package)
    {
        uint8_t i = 0;
        while (i < length)
        {
            *(buffer + i) = lcm_get_data();
            i++;
        }
        return i;
    }
    return 0;
}

uint16_t lcm_get_data()
{
    if (LCM_RX_PACKAGE_NULL != g_lcm_rx_package)
    {
        uint16_t data = 0;
        switch (g_lcm_rx_package->rx_type)
        {
        case LCM_READ_REGISTER_COMMAND:
        {
            data = *(g_lcm_rx_package->p_data_buffer
                + g_lcm_rx_package->data_buffer_index);
        }
        break;
        case LCM_READ_VP_COMMAND:
        {
            data = lcm_make16(
                *(g_lcm_rx_package->p_data_buffer
                    + (g_lcm_rx_package->data_buffer_index << 1)),
                *(g_lcm_rx_package->p_data_buffer
                    + ((g_lcm_rx_package->data_buffer_index << 1) + 1)));
        }
        break;
        default:
            break;
        }

        g_lcm_rx_package->data_buffer_index++;
        if (g_lcm_rx_package->data_buffer_index >= g_lcm_rx_package->rx_length)
        {
            g_lcm_rx_package->data_buffer_index = 0;
            g_lcm_rx_package->rx_length = 0;
            g_lcm_rx_package = LCM_RX_PACKAGE_NULL;
            lcm_queue_garbage_collector();
        }
        return data;
    }
    return 0;
}

uint8_t lcm_get_rx_type()
{
    return g_lcm_rx_package->rx_type;
}

uint16_t lcm_get_rx_address()
{
    return g_lcm_rx_package->rx_address;
}

uint8_t lcm_get_rx_length()
{
    return g_lcm_rx_package->rx_length;
}

void lcm_change_brightness(uint8_t value)
{
    uint16_t brightness = value;

    if (brightness > 100)
    {
        brightness = 100;
    }

    brightness = brightness * 63;
    brightness = brightness / 100;

    value = brightness;

    lcm_write_multiple_registers(0x01, &value, 1);
}

void lcm_beep_buzzer(uint8_t value)
{
    lcm_write_multiple_registers(0x02, &value, 1);
}

void lcm_change_pic_id(uint16_t value)
{
    uint8_t* buffer = (uint8_t*)malloc(2u);

    *(buffer) = lcm_high_byte_of_word(value);
    *(buffer + 1) = lcm_low_byte_of_word(value);

    lcm_write_multiple_registers(0x03, buffer, 2);
    free(buffer);
}

void lcm_write_rtc(lcm_rtc_datetime_t lcm_rtc_datetime)
{
    lcm_update_data();

    uint8_t* buffer = (uint8_t*)malloc(8u);

    *(buffer) = g_lcm_register_activate_command;
    *(buffer + 1) = lcm_int_to_bcd(((uint8_t)(lcm_rtc_datetime.year - 2000)));
    *(buffer + 2) = lcm_int_to_bcd(lcm_rtc_datetime.month);
    *(buffer + 3) = lcm_int_to_bcd(lcm_rtc_datetime.day);
    *(buffer + 4) = lcm_int_to_bcd(lcm_rtc_datetime.week);
    *(buffer + 5) = lcm_int_to_bcd(lcm_rtc_datetime.hour);
    *(buffer + 6) = lcm_int_to_bcd(lcm_rtc_datetime.minute);
    *(buffer + 7) = lcm_int_to_bcd(lcm_rtc_datetime.second);

    lcm_write_multiple_registers(0x1F, buffer, 8u);
    free(buffer);
}

void lcm_rtc_write_year(uint16_t value)
{
    g_lcm_rtc.year = value;
    lcm_write_rtc(g_lcm_rtc);
}

void lcm_rtc_write_month(uint8_t value)
{
    g_lcm_rtc.month = value;
    lcm_write_rtc(g_lcm_rtc);
}

void lcm_rtc_write_day(uint8_t value)
{
    g_lcm_rtc.day = value;
    lcm_write_rtc(g_lcm_rtc);
}

void lcm_rtc_write_hours(uint8_t value)
{
    g_lcm_rtc.hour = value;
    lcm_write_rtc(g_lcm_rtc);
}

void lcm_rtc_write_minutes(uint8_t value)
{
    g_lcm_rtc.minute = value;
    lcm_write_rtc(g_lcm_rtc);
}

void lcm_rtc_write_seconds(uint8_t value)
{
    g_lcm_rtc.second = value;
    lcm_write_rtc(g_lcm_rtc);
}

void lcm_rtc_write_date(uint16_t day, uint8_t month, uint8_t year)
{
    g_lcm_rtc.year = year;
    g_lcm_rtc.month = month;
    g_lcm_rtc.day = day;
    lcm_write_rtc(g_lcm_rtc);
}

void lcm_rtc_write_time(uint16_t hours, uint8_t minutes, uint8_t seconds)
{
    g_lcm_rtc.hour = hours;
    g_lcm_rtc.minute = minutes;
    g_lcm_rtc.second = seconds;
    lcm_write_rtc(g_lcm_rtc);
}

void lcm_activate_software_control(uint8_t software_control_code)
{
    lcm_write_multiple_registers(0x4F, &software_control_code, 1);
}

void lcm_play_audio(uint16_t id, uint8_t volume)
{
    uint16_t volume_value = (uint16_t)volume << 6;
    volume_value /= 100;

    uint8_t* buffer = (uint8_t*)malloc(5u);

    *(buffer + 0) = PLAY;
    *(buffer + 1) = lcm_high_byte_of_word(id);
    *(buffer + 2) = lcm_low_byte_of_word(id);
    *(buffer + 3) = g_lcm_register_activate_command;
    *(buffer + 4) = volume_value;

    lcm_write_multiple_registers(0x50, buffer, 5);
    free(buffer);
}

void lcm_stop_audio(uint16_t id)
{
    uint8_t* buffer = (uint8_t*)malloc(3u);

    *(buffer + 0) = PAUSE;
    *(buffer + 1) = lcm_high_byte_of_word(id);
    *(buffer + 2) = lcm_low_byte_of_word(id);

    lcm_write_multiple_registers(0x50, buffer, 3);
    free(buffer);
}

void lcm_write_video_settings(uint16_t id, lcm_video_mode_t lcm_video_mode,
    uint8_t volume, uint16_t x, uint16_t y)
{
    uint8_t volume_value = ((volume * 0x3F) / 100);
    uint8_t* buffer = (uint8_t*)malloc(10u);

    *(buffer + 0) = g_lcm_register_activate_command;
    *(buffer + 1) = lcm_video_mode;
    *(buffer + 2) = lcm_high_byte_of_word(x);
    *(buffer + 3) = lcm_low_byte_of_word(x);
    *(buffer + 4) = lcm_high_byte_of_word(y);
    *(buffer + 5) = lcm_low_byte_of_word(y);
    *(buffer + 6) = lcm_high_byte_of_word(id);
    *(buffer + 7) = lcm_low_byte_of_word(id);
    *(buffer + 8) = g_lcm_register_activate_command;
    *(buffer + 9) = volume_value;

    lcm_write_multiple_registers(0x60, buffer, 10);
    free(buffer);
}

void lcm_play_pause_video()
{
    lcm_write_register(0x6A, g_lcm_register_activate_command);
}

void lcm_stop_video()
{
    lcm_write_register(0x6B, g_lcm_register_activate_command);
}

void lcm_next_video()
{
    lcm_write_register(0x6C, g_lcm_register_activate_command);
}

void lcm_previous_video()
{
    lcm_write_register(0x6D, g_lcm_register_activate_command);
}

void lcm_start_calibration()
{
    lcm_write_register(0xEA, g_lcm_register_activate_command);
}

uint8_t lcm_clear_trend_curve(lcm_trend_curve_channel_t lcm_trend_curve_channel)
{
    return lcm_write_register(0xEB, 0x56 + lcm_trend_curve_channel);
}

void lcm_write_trend_curve(uint16_t* value,
    lcm_trend_curve_channel_t lcm_trend_curve_channel, uint8_t length)
{
    
    uint8_t byte_count = (length << 1) + 2;
    uint8_t total_length = byte_count + 3;
    uint8_t pre_buffer_length = 5;

    uint8_t* buffer = (uint8_t*)malloc(total_length);

    *(buffer + 0) = g_lcm_header_high_byte;
    *(buffer + 1) = g_lcm_header_low_byte;
    *(buffer + 2) = byte_count;
    *(buffer + 3) = LCM_WRITE_TREND_CURVE_COMMAND;
    *(buffer + 4) = lcm_trend_curve_channel;

    for (uint8_t i = 0; i < length; i++)
    {
        *(buffer + (i << 1) + pre_buffer_length) = lcm_high_byte_of_word(
            *(value + i));
        *(buffer + ((i << 1) + 1) + pre_buffer_length) = lcm_low_byte_of_word(
            *(value + i));
    }

    lcm_update_data();

    lcm_write_out_bytes(buffer, total_length);

    /**********************************************************************************************************
    free(buffer);*/
}

void lcm_reset_lcm()
{
    uint8_t* buffer = (uint8_t*)malloc(2u);

    *(buffer + 0) = g_lcm_register_activate_command;
    *(buffer + 1) = 0xA5;

    lcm_write_multiple_registers(0xEE, buffer, 2u);
    free(buffer);
}

uint8_t lcm_read_firmware_version()
{
    return lcm_read_register(0x00);
}

uint16_t lcm_read_pic_id()
{
    uint8_t* buffer = (uint8_t*)malloc(2u);

    uint16_t pic_id = 0xFFFF;

    if (lcm_read_multiple_registers(0x03, buffer, 2) > 0)
    {
        pic_id = lcm_make16(*(buffer + 0), *(buffer + 1));
    }

    free(buffer);

    return pic_id;
}

uint8_t lcm_read_audio_status()
{
    return lcm_read_register(0x55);
}

uint8_t lcm_read_video_status()
{
    return lcm_read_register(0x6E);
}

uint8_t lcm_read_keyboard_status()
{
    return lcm_read_register(0xE9);
}

void lcm_write_trend_curve0(uint16_t value)
{
    lcm_write_trend_curve(&value, CHANNEL_0, 1u);
}
void lcm_write_trend_curve1(uint16_t value)
{
    lcm_write_trend_curve(&value, CHANNEL_1, 1u);
}
void lcm_write_trend_curve2(uint16_t value)
{
    lcm_write_trend_curve(&value, CHANNEL_2, 1u);
}
void lcm_write_trend_curve3(uint16_t value)
{
    lcm_write_trend_curve(&value, CHANNEL_3, 1u);
}
void lcm_write_trend_curve4(uint16_t value)
{
    lcm_write_trend_curve(&value, CHANNEL_4, 1u);
}
void lcm_write_trend_curve5(uint16_t value)
{
    lcm_write_trend_curve(&value, CHANNEL_5, 1u);
}
void lcm_write_trend_curve6(uint16_t value)
{
    lcm_write_trend_curve(&value, CHANNEL_6, 1u);
}
void lcm_write_trend_curve7(uint16_t value)
{
    lcm_write_trend_curve(&value, CHANNEL_7, 1u);
}

uint8_t lcm_clear_trend_curve0()
{
    return lcm_clear_trend_curve(0);
}
uint8_t lcm_clear_trend_curve1()
{
    return lcm_clear_trend_curve(1);
}
uint8_t lcm_clear_trend_curve2()
{
    return lcm_clear_trend_curve(2);
}
uint8_t lcm_clear_trend_curve3()
{
    return lcm_clear_trend_curve(3);
}
uint8_t lcm_clear_trend_curve4()
{
    return lcm_clear_trend_curve(4);
}
uint8_t lcm_clear_trend_curve5()
{
    return lcm_clear_trend_curve(5);
}
uint8_t lcm_clear_trend_curve6()
{
    return lcm_clear_trend_curve(6);
}
uint8_t lcm_clear_trend_curve7()
{
    return lcm_clear_trend_curve(7);
}

uint8_t lcm_read_rtc()
{
    uint8_t* buffer = (uint8_t*)malloc(7u);

    if (lcm_read_multiple_registers(0x20, buffer, 7) > 0)
    {
        g_lcm_rtc.year = lcm_bcd_to_int(*(buffer + 0)) + 2000;
        g_lcm_rtc.month = lcm_bcd_to_int(*(buffer + 1));
        g_lcm_rtc.day = lcm_bcd_to_int(*(buffer + 2));
        g_lcm_rtc.week = *(buffer + 3);
        g_lcm_rtc.hour = lcm_bcd_to_int(*(buffer + 4));
        g_lcm_rtc.minute = lcm_bcd_to_int(*(buffer + 5));
        g_lcm_rtc.second = lcm_bcd_to_int(*(buffer + 6));
        free(buffer);

        return 1;
    }

    free(buffer);
    return 0;
}

uint16_t lcm_rtc_get_year()
{
    return g_lcm_rtc.year;
}
uint8_t lcm_rtc_get_month()
{
    return g_lcm_rtc.month;
}
uint8_t lcm_rtc_get_day()
{
    return g_lcm_rtc.day;
}
uint8_t lcm_rtc_get_day_of_the_week()
{
    return g_lcm_rtc.week;
}
uint8_t lcm_rtc_get_hours()
{
    return g_lcm_rtc.hour;
}
uint8_t lcm_rtc_get_minutes()
{
    return g_lcm_rtc.minute;
}
uint8_t lcm_rtc_get_seconds()
{
    return g_lcm_rtc.second;
}

/*****************************************************************************************
 * LCMVAR
 */

uint8_t lcm_var_available(lcm_var_parameters_t* lcm_var_parameters)
{
    lcm_update_data();
    /*
       if (lcm_NULL == lcm_var_parameters->p_rx_package)
       {
          lcm_var_parameters->p_rx_package = lcm_rx_package_null;
       }
    */
	
    if (LCM_RX_PACKAGE_NULL == lcm_var_parameters->p_rx_package)
    {
        lcm_var_parameters->p_rx_package = lcm_queue_search(
			g_lcm_read_vp_command,
            lcm_var_parameters->vp);
    }

    if (LCM_RX_PACKAGE_NULL != lcm_var_parameters->p_rx_package)
    {
        if (lcm_var_parameters->p_rx_package->garbage_collector_counter > g_lcm_garbage_collector_max_ticks)
        {
            lcm_var_parameters->p_rx_package = LCM_RX_PACKAGE_NULL;
            return 0;
        }

        return lcm_var_parameters->p_rx_package->rx_length
            - lcm_var_parameters->p_rx_package->data_buffer_index;
    }
    return 0;
}

static uint16_t lcm_var_get_data_in(lcm_var_parameters_t* parameters)
{
    uint16_t data =
        lcm_make16(
            *(parameters->p_rx_package->p_data_buffer
                + (parameters->p_rx_package->data_buffer_index << 1)),
            *(parameters->p_rx_package->p_data_buffer
                + ((parameters->p_rx_package->data_buffer_index << 1)
                    + 1)));

    parameters->p_rx_package->data_buffer_index++;

    if (parameters->p_rx_package->data_buffer_index
        >= parameters->p_rx_package->rx_length)
    {
        parameters->p_rx_package->data_buffer_index = 0;
        parameters->p_rx_package->rx_length = 0;
        lcm_queue_garbage_collector();
        parameters->p_rx_package = LCM_RX_PACKAGE_NULL;
    }

    return data;
}


#if (defined(UINT64_MAX))
uint64_t lcm_var_get_data(lcm_var_parameters_t* lcm_var_parameters)
#else
uint32_t lcm_var_get_data(lcm_var_parameters_t* lcm_var_parameters)
#endif
{

    if (lcm_var_available(lcm_var_parameters) > 0)
    {
#if (defined(UINT64_MAX))
        uint64_t data = 0;
#else
        uint32_t data = 0;
#endif

        switch (lcm_var_parameters->vp_mode)
        {
        case LOW_BYTE:
        case HIGH_BYTE:
        {
            if (lcm_var_parameters->vp_mode == LOW_BYTE)
            {
                data = (lcm_var_get_data_in(lcm_var_parameters) & 0x00FF);
            }
            else
            {
                data =
                    ((lcm_var_get_data_in(lcm_var_parameters) & 0xFF00) >> 8);
            }
            break;
        }
        case TWO_BYTES:
        case TWO_BYTES_UNSIGNED:
        {
            data = lcm_var_get_data_in(lcm_var_parameters);

            break;
        }
        case FOUR_BYTES:
        case FOUR_BYTES_UNSIGNED:
        {
            data = lcm_var_get_data_in(lcm_var_parameters);
            data = data << 16;
            data = data | lcm_var_get_data_in(lcm_var_parameters);

            break;
        }
        case EIGHT_BYTES:
        {
#if (defined(UINT64_MAX))
            uint64_t dataAux = 0;

            dataAux = lcm_var_get_data_in(lcm_var_parameters);
            data = dataAux << 48;

            dataAux = lcm_var_get_data_in(lcm_var_parameters);
            data |= (dataAux << 32);

#else
            uint32_t dataAux = 0;
            dataAux = lcm_var_get_data_in(lcm_var_parameters);
            dataAux = lcm_var_get_data_in(lcm_var_parameters);
#endif


            dataAux = lcm_var_get_data_in(lcm_var_parameters);
            data |= (dataAux << 16);

            dataAux = lcm_var_get_data_in(lcm_var_parameters);
            data |= dataAux;

            break;
        }
        default:
            break;
        }

        return data;
    }
    return 0;
}

#if (defined(UINT64_MAX))
uint64_t lcm_var_read(lcm_var_parameters_t* lcm_var_parameters)
#else
uint32_t lcm_var_read(lcm_var_parameters_t* lcm_var_parameters)
#endif
{

#if (defined(UINT64_MAX))
    uint64_t data = 0;
#else
    uint32_t data = 0;
#endif

    switch (lcm_var_parameters->vp_mode)
    {
    case LOW_BYTE:
    case HIGH_BYTE:
    case TWO_BYTES:
    case TWO_BYTES_UNSIGNED:
    {
        data = lcm_read_vp(lcm_var_parameters->vp);
        break;
    }
    case FOUR_BYTES:
    case FOUR_BYTES_UNSIGNED:
    {
        uint16_t* datas = (uint16_t*)malloc(2u);

        lcm_read_multiple_vps(lcm_var_parameters->vp, datas, 2u);

        data = (uint32_t)(*(datas + 0) << 16) | *(datas + 1);

        free(datas);

        break;
    }
    case EIGHT_BYTES:
    {
        uint16_t datas[4];
        lcm_read_multiple_vps(lcm_var_parameters->vp, datas, 4);

#if (defined(UINT64_MAX))
        uint64_t datas64[4];
#else
        uint32_t datas64[4];
#endif

        datas64[0] = datas[0];
        datas64[1] = datas[1];
        datas64[2] = datas[2];
        datas64[3] = datas[3];

#if (defined(UINT64_MAX))
        data = (uint64_t)((datas64[0] << 48) | (datas64[1] << 32) | (datas64[2] << 16) | datas64[3]);
#else
        data = (uint32_t)((datas64[0] << 48) | (datas64[1] << 32) | (datas64[2] << 16) | datas64[3]);
#endif
        break;
    }
    default:
        break;
    }
    return data;
}

uint8_t lcm_var_request(lcm_var_parameters_t* lcm_var_parameters)
{
    uint8_t status = 0;
    switch (lcm_var_parameters->vp_mode)
    {
    case LOW_BYTE:
    case HIGH_BYTE:
    case TWO_BYTES:
    case TWO_BYTES_UNSIGNED:
    {
        status = lcm_request_vp(lcm_var_parameters->vp, 1u);
        break;
    }
    case FOUR_BYTES:
    case FOUR_BYTES_UNSIGNED:
    {
        status = lcm_request_vp(lcm_var_parameters->vp, 2u);
        break;
    }
    case EIGHT_BYTES:
    {
        status = lcm_request_vp(lcm_var_parameters->vp, 4u);
        break;
    }
    default:
        break;
    }
    return status;
}

static uint8_t lcm_var_parse_output_and_write_16(
    lcm_var_parameters_t* parameters, uint16_t data)
{
    return lcm_write_multiple_vps(parameters->vp, &data, 1u);
}

static uint8_t lcm_var_parse_output_and_write_32(
    lcm_var_parameters_t* parameters, uint32_t data)
{
    uint16_t* output16 = (uint16_t*)malloc(sizeof(uint16_t) * 2u);

    *(output16 + 0) = data >> 16;
    *(output16 + 1) = data;

    uint8_t status = lcm_write_multiple_vps(parameters->vp,
        (uint16_t*)output16, 2u);
    free(output16);
    return status;
}

uint8_t lcm_var_write(lcm_var_parameters_t* lcm_var_parameters, uint32_t value)
{

    switch (lcm_var_parameters->vp_mode)
    {
    case TWO_BYTES_UNSIGNED:
    case TWO_BYTES:
    {
        return lcm_var_parse_output_and_write_16(lcm_var_parameters, value);
    }
    break;
    case FOUR_BYTES_UNSIGNED:
    case FOUR_BYTES:
    {
        return lcm_var_parse_output_and_write_32(lcm_var_parameters, value);
    }
    break;
    default:
        return g_lcm_null;
        break;
    }
}

#if (defined(UINT64_MAX))
static uint8_t lcm_var_parse_output_and_write_64(lcm_var_parameters_t* parameters, uint64_t data)
{
    uint8_t return_status = 0;

    uint16_t output16[] =
    {
    data >> 48,
    data >> 32,
    data >> 16,
    data,
    };

    return_status = lcm_write_multiple_vps(parameters->vp, output16, 4u);

    return return_status;
}

uint8_t lcm_var_write64(lcm_var_parameters_t* lcm_var_parameters, uint64_t value)
{
    if (EIGHT_BYTES == lcm_var_parameters->vp_mode)
    {
        return lcm_var_parse_output_and_write_64(lcm_var_parameters, value);
    }
    else
    {
        return g_lcm_null;
    }
}
#endif


/******************************************************************************************************************************
 * LcmString
 */

uint8_t lcm_string_available(lcm_string_parameters_t* lcm_string_parameters)
{
    lcm_update_data();

    uint8_t rx_length_char = 0;
    /*
       if (lcm_NULL == lcm_string_parameters->p_rx_package)
       {
          lcm_string_parameters->p_rx_package = lcm_rx_package_null;
       }
    */
    if (LCM_RX_PACKAGE_NULL == lcm_string_parameters->p_rx_package)
    {
        lcm_string_parameters->p_rx_package = lcm_queue_search( g_lcm_read_vp_command,
            lcm_string_parameters->vp);

        if (LCM_RX_PACKAGE_NULL != lcm_string_parameters->p_rx_package)
        {
            lcm_string_parameters->p_rx_package->rx_length =
                (lcm_string_parameters->p_rx_package->rx_length - 1) << 1;

            for (uint8_t i = 0;
                i < lcm_string_parameters->p_rx_package->rx_length; i++)
            {
                if ((g_lcm_string_terminator
                    == *(lcm_string_parameters->p_rx_package->p_data_buffer
                        + i))
                    || (0
                        == *(lcm_string_parameters->p_rx_package->p_data_buffer
                            + i)))
                {
                    lcm_string_parameters->p_rx_package->rx_length -= 1;
                    break;
                }
            }
            rx_length_char = (lcm_string_parameters->p_rx_package->rx_length
                - lcm_string_parameters->p_rx_package->data_buffer_index);
        }
    }
    else
    {
        rx_length_char = (lcm_string_parameters->p_rx_package->rx_length
            - lcm_string_parameters->p_rx_package->data_buffer_index);
    }

    if (rx_length_char == 0)
    {
        if (LCM_RX_PACKAGE_NULL != lcm_string_parameters->p_rx_package)
        {
            lcm_string_parameters->p_rx_package->data_buffer_index = 0;
            lcm_string_parameters->p_rx_package->rx_length = 0;
            lcm_queue_garbage_collector();
            lcm_string_parameters->p_rx_package = LCM_RX_PACKAGE_NULL;
        }
    }

    return rx_length_char;
}

uint8_t lcm_string_get_data(lcm_string_parameters_t* lcm_string_parameters)
{
    uint8_t data = 0;
    if (0 != lcm_string_available(lcm_string_parameters))
    {
        data = *(lcm_string_parameters->p_rx_package->p_data_buffer
            + lcm_string_parameters->p_rx_package->data_buffer_index);

        lcm_string_parameters->p_rx_package->data_buffer_index++;

        if (lcm_string_parameters->p_rx_package->rx_length
            == lcm_string_parameters->p_rx_package->data_buffer_index)
        {
            lcm_string_parameters->p_rx_package->data_buffer_index = 0;
            lcm_string_parameters->p_rx_package->rx_length = 0;
            lcm_queue_garbage_collector();
            lcm_string_parameters->p_rx_package = LCM_RX_PACKAGE_NULL;
        }
    }
    return data;
}

uint8_t lcm_string_request(lcm_string_parameters_t* lcm_string_parameters)
{
    return lcm_request_vp(lcm_string_parameters->vp,
        lcm_string_parameters->length_in_bytes);
}

uint8_t lcm_string_write(lcm_string_parameters_t* lcm_string_parameters,
    uint8_t* chars, uint16_t length)
{
    if (length > lcm_string_parameters->length_in_bytes)
    {
        length = lcm_string_parameters->length_in_bytes;
    }

    uint8_t copy_length = (length >> 1);
    uint8_t vp_length = (copy_length + 2);

    uint16_t* data16 = (uint16_t*)malloc(vp_length * sizeof(uint16_t));

    for (uint8_t i = 0; i < copy_length; i++)
    {
        *(data16 + i) = lcm_make16(*(chars + (i << 1)),
            *(chars + ((i << 1) + 1)));
    }

    if ((length % 2) == 0)
    {
        vp_length--;
    }
    else
    {
        *(data16 + (vp_length - 2)) = lcm_make16(*(chars + (length - 1)), 0xFF);
    }

    *(data16 + (vp_length - 1)) = 0xFFFF;

    uint8_t return_status = 0;

    return_status = lcm_write_multiple_vps(lcm_string_parameters->vp, data16,
        vp_length);

    free(data16);

    return return_status;
}

/**********************************************************************************
 * initialization
 */

void lcm_init()
{
    lcm_queue_new_queue();
}

void lcm_crc_enable()
{
    g_lcm_use_crc = LCM_CRC_TRUE;
}

void lcm_crc_disable()
{
    g_lcm_use_crc = LCM_CRC_FALSE;
}

void lcm_set_read_timeout(uint32_t timeout_in_ticks)
{
    g_lcm_read_timeout = timeout_in_ticks;
}

void lcm_disable_garbage_collector()
{
    g_lcm_garbage_collector_max_ticks = 0;
}

void lcm_enable_garbage_collector()
{
    g_lcm_garbage_collector_max_ticks = 500000;
}

void lcm_set_header(uint16_t header)
{
    g_lcm_header_high_byte = header >> 8;
    g_lcm_header_low_byte = header;
}

/**********************************************************************************
 * project transfer
 */

static uint8_t  pleaseTxFileReceivedByteIndex = 0;

static uint8_t PleaseTxFileIsReceived()
{
    static char *pleaseTxFileWord = "Please Tx file !";

    uint16_t parse_byte = lcm_read_byte();

    if (g_lcm_null != parse_byte)
    {
        if (pleaseTxFileWord[pleaseTxFileReceivedByteIndex] == (char)(parse_byte & 0xFF))
        {
            if (pleaseTxFileWord[pleaseTxFileReceivedByteIndex] == '!')
            {
                pleaseTxFileReceivedByteIndex = 0;
                return 1;
            }
            ++pleaseTxFileReceivedByteIndex;
        }
        else
        {
            pleaseTxFileReceivedByteIndex = 0;
        }
    }
    return 0;
}

uint8_t transfer_file_begin(const char* name, uint32_t name_size, uint32_t file_size)
{
    char _command[50];
    uint8_t pleaseTxFileIsReceived = 0;

    _command[0] = g_lcm_header_high_byte;
    _command[1] = g_lcm_header_low_byte;

    _command[2] = name_size + 8;

    _command[3] = 0x85;
    _command[4] = 0x01;

    _command[5] = 0x5A;
    _command[6] = 0xA5;

    _command[10] = file_size & 0xff;
    file_size = file_size >> 8;
    _command[9] = file_size & 0xff;
    file_size = file_size >> 8;
    _command[8] = file_size & 0xff;
    file_size = file_size >> 8;
    _command[7] = file_size & 0xff;



    memcpy(_command + 11, name, name_size);

    uint32_t transferLength = name_size + 11;

    lcm_write_bytes(_command, transferLength);

    pleaseTxFileReceivedByteIndex = 0;

    uint32_t wait_ticks = 0;
    while (wait_ticks < g_lcm_read_timeout)
    {
        if (PleaseTxFileIsReceived())
        {
            pleaseTxFileIsReceived = 1;
            break;
        }
        ++wait_ticks;
    }

    return pleaseTxFileIsReceived;
}

/*** end of file ***/
