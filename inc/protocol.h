/*
 * =====================================================================
 *
 *          @file:  protocol.h
 *
 *        @author:  dengos (), dengos.w@gmail.com
 *         @brief:  
 *
 *       Revision:  none
 *       Compiler:  gcc
 *
 *        Company:  scut
 *          @date:  06/13/2012 06:55:38 PM
 *       @version:  1.0
*
 * =====================================================================
 */




/**
 * @brief Specific the type of given command. The command objects
 * are the protocol between dsp image server and pc python image client.
 * Notice: when using command, please beware that there are no one but 
 * yourself, you have to consistance the image or video frame size between
 * dsp image server and pc python image client.
 *
 */
enum CommandType 
{

    /**
     * @brief command type for test.
     */
    CommandTypeTest = 1,  
    /**
     * @brief command for video processing. When client try to send a video
     * command to server, there must follow by a stream of video frame, which
     * must be interpreted into yuv format.
     */
    CommandTypeVideo,

    /**
     * @brief command for image segmentation. Segment command must follow by a 
     * gray scale image. 
     */
    CommandTypeSegment,
};				
/* ----------  end of enum command_type  ---------- */



/**
 * @brief The command struct, kind of base for more specific commands.
 */
struct command {
    int             id; 
    CommandType     cmd;  
};	
/* ----------  end of struct command  ---------- */




/**
 * @brief Also a kind of command, but data command is more like a data container, which
 * leader by other kind of processing command.
 * The main function of data command are identity between different data command, and
 * specific how many data is going to send.
 */
struct data_command 
{
    int             id; 
    CommandType     cmd; 
    int             data_size; 
}; 
/* ----------  end of struct struct data_command  ---------- */


/**
 * @brief A kind of processing command. In video command, we specific how many
 * frames are going to send, and what's the frame size.
 */
struct video_command 
{
    int             id; 
    CommandType     cmd; 
    int             frame_num; 
    int             width; 
    int             height; 
}; 
/* ----------  end of struct video_command  ---------- */


/**
 * @brief A kind of processing command. In segmentation command, we spcific the
 * size of given image. Do notice that, the image must be a gray scale image.
 */
struct segment_command 
{
    int             id;
    CommandType     cmd;
    int             width;
    int             height;
};				
/* ----------  end of struct segment_command  ---------- */


/**
 * @brief a data struct using only in dsp server end.
 */
struct data {
    int     size; 
    char    *pdata; 
}; 
/* ----------  end of struct data  ---------- */

