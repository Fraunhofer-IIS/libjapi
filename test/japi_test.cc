#include <gtest/gtest.h>
#include <stdbool.h>

extern "C"{
#include <japi.h>
#include <japi_intern.h>
#include <japi_pushsrv_intern.h>
#include <japi_pushsrv.h>
#include <japi_utils.h>
#include <rw_n.h>
}

/* The handler for japi_register_request test */
static void dummy_request_handler(japi_context *ctx, json_object *request, json_object *response)
{
	/* Not existent dummy request */
	json_object_object_add(response,"value",json_object_new_string("hello world"));
}

TEST(JAPI,Init)
{
	/* On success, a japi_context object is returned. On error, NULL is returned */
	EXPECT_TRUE(japi_init(NULL) != NULL);
}

TEST(JAPI,GetValueAsX)
{
	bool bval;
	const char* sval;
	int ival;
	long long int lval;
	double dval;
	json_object *jresp;

	jresp = json_object_new_object();
	json_object_object_add(jresp,"string",json_object_new_string("value"));
	json_object_object_add(jresp,"bool",json_object_new_boolean(true));
	json_object_object_add(jresp,"int",json_object_new_int(10));
	json_object_object_add(jresp,"int64",json_object_new_int64(9000000000000000000));
	json_object_object_add(jresp,"double",json_object_new_double(10.12345));

	/* On success, string is returned. On error, <0 is returned */
	EXPECT_EQ(japi_get_value_as_str(jresp, "string", &sval), 0);
	EXPECT_STREQ(sval,"value");
	/* On success, bool is returned. On error, <0 is returned */
	EXPECT_EQ(japi_get_value_as_bool(jresp, "bool",&bval),0);
	EXPECT_TRUE(bval);
	/* On success, int is returned. On error, <0 is returned */
	EXPECT_EQ(japi_get_value_as_int(jresp, "int",&ival),0);
	EXPECT_EQ(ival,10);
	/* On success, int64 is returned. On error, <0 is returned */
	EXPECT_EQ(japi_get_value_as_int64(jresp, "int64",&lval),0);
	EXPECT_EQ(lval,9000000000000000000);
	/* On success, double is returned. On error, <0 is returned */
	EXPECT_EQ(japi_get_value_as_double(jresp, "double",&dval),0);
	EXPECT_EQ(dval,10.12345);

	/* Test error return values */
	/* Given json-object is NULL, expecting -1 */
	EXPECT_EQ(japi_get_value_as_str(NULL, "string", &sval), -1);
	EXPECT_EQ(japi_get_value_as_bool(NULL, "bool", &bval), -1);
	EXPECT_EQ(japi_get_value_as_int(NULL, "int", &ival), -1);
	EXPECT_EQ(japi_get_value_as_int64(NULL, "int64", &lval), -1);
	EXPECT_EQ(japi_get_value_as_double(NULL, "double", &dval), -1);
	/* Given key is NULL, expecting -2 */
	EXPECT_EQ(japi_get_value_as_str(jresp, NULL, &sval), -2);
	EXPECT_EQ(japi_get_value_as_bool(jresp, NULL, &bval), -2);
	EXPECT_EQ(japi_get_value_as_int(jresp, NULL, &ival), -2);
	EXPECT_EQ(japi_get_value_as_int64(jresp, NULL, &lval), -2);
	EXPECT_EQ(japi_get_value_as_double(jresp, NULL, &dval), -2);
	/* Given doesn't exist, expecting -3 */
	EXPECT_EQ(japi_get_value_as_str(jresp, "NotExistingKey", &sval), -3);
	EXPECT_EQ(japi_get_value_as_bool(jresp, "NotExistingKey", &bval), -3);
	EXPECT_EQ(japi_get_value_as_int(jresp, "NotExistingKey", &ival), -3);
	EXPECT_EQ(japi_get_value_as_int64(jresp, "NotExistingKey", &lval), -3);
	EXPECT_EQ(japi_get_value_as_double(jresp, "NotExistingKey", &dval), -3);
	/* Requested value doesn't have the respective type, expecting -4 */
	EXPECT_EQ(japi_get_value_as_str(jresp, "bool", &sval), -4);
	EXPECT_EQ(japi_get_value_as_bool(jresp, "string", &bval), -4);
	EXPECT_EQ(japi_get_value_as_int(jresp, "string", &ival), -4);
	EXPECT_EQ(japi_get_value_as_int64(jresp, "string", &lval), -4);
	EXPECT_EQ(japi_get_value_as_double(jresp, "string", &dval), -4);

	/* Clean up */
	json_object_put(jresp);
}

TEST(JAPI,ProcessMessage)
{
	japi_context *ctx;
	char* response;
	const char* request;
	const char* sval;
	json_object *jobj;
	json_object *jdata;
	int socket;

	jobj = json_object_new_object();
	jdata= json_object_new_object();
	request = "{'japi_request':'dummy_request_handler'}";
	response = NULL;
	ctx = japi_init(NULL);
	socket = 4;

	japi_register_request(ctx,"dummy_request_handler",&dummy_request_handler);
	/* On success, 0 returned. On error, -1 is returned */
	EXPECT_EQ(japi_process_message(ctx, request, &response, socket),0);
	jobj = json_tokener_parse(response);
	json_object_object_get_ex(jobj,"data",&jdata);
	EXPECT_EQ(japi_get_value_as_str(jdata,"value",&sval),0);
	EXPECT_STREQ("hello world",sval);

	/* Clean up */
	japi_destroy(ctx);
}

TEST(JAPI,IncludeArgsWithResponse)
{
	/* Setup */
	japi_context *ctx = japi_init(NULL);
	char* response = NULL;
	const char* sval;
	const char* request = "{'japi_request': 'dummy_request_handler', 'args': {'foo': 'bar'}}";
	const char* request_int_args = "{'japi_request': 'dummy_request_handler', 'args': 42}";
	json_object *jobj;
	json_object *jdata;
	int socket = 4;

	/* Configure context to include request arguments in response */
	EXPECT_EQ(japi_include_args_in_response(NULL, false), -1);
	EXPECT_EQ(japi_include_args_in_response(ctx, false), 0);
	EXPECT_EQ(japi_include_args_in_response(ctx, true), 0);

	/* Register dummy request handler */
	japi_register_request(ctx,"dummy_request_handler",&dummy_request_handler);

	/* Response should include request arguments object */
	EXPECT_EQ(japi_process_message(ctx, request, &response, socket),0);
	jobj = json_tokener_parse(response);
	EXPECT_TRUE(json_object_object_get_ex(jobj, "args", &jdata));
	EXPECT_EQ(japi_get_value_as_str(jdata,"foo",&sval),0);
	EXPECT_STREQ("bar", sval);

	/* Response should include request argument integer */
	EXPECT_EQ(japi_process_message(ctx, request_int_args, &response, socket),0);
	jobj = json_tokener_parse(response);
	EXPECT_TRUE(json_object_object_get_ex(jobj, "args", &jdata));
	EXPECT_EQ(42, json_object_get_int(jdata));
	
	/* Teardown */
	japi_destroy(ctx);
}

TEST(JAPI,Register)
{
	japi_context *ctx;
	ctx = japi_init(NULL);

	/* On success, zero is returned. On error, -1..-4 is returned */
	EXPECT_EQ(japi_register_request(ctx,"req_name",&dummy_request_handler),0);
	EXPECT_EQ(japi_register_request(NULL,"req_name",&dummy_request_handler),-1);
	EXPECT_EQ(japi_register_request(ctx,NULL,&dummy_request_handler),-2);
	EXPECT_EQ(japi_register_request(ctx,"req_name",NULL),-3);

	/* Registering the same request name again or an empty request name, should not be possible */
	EXPECT_EQ(japi_register_request(ctx,"req_name",&dummy_request_handler),-4);
	EXPECT_EQ(japi_register_request(ctx,"dummy_request_02",&dummy_request_handler),0); // same handler for another name
	EXPECT_EQ(japi_register_request(ctx,"",&dummy_request_handler),-2);

	japi_destroy(ctx);
}

TEST(JAPI_Push_Service,Register)
{
	japi_context *ctx;
	ctx = japi_init(NULL);

	/* On success, a pointer to the japi_push_service context is returned. On error, NULL is returned */
	EXPECT_TRUE(japi_pushsrv_register(ctx,"test_pushsrv") != NULL);
	EXPECT_TRUE(japi_pushsrv_register(NULL,"test_pushsrv") == NULL);
	EXPECT_TRUE(japi_pushsrv_register(ctx,NULL) == NULL);

	/* Registering the same push service name again or an empty push service name, should not be possible */
	EXPECT_TRUE(japi_pushsrv_register(ctx,"test_pushsrv") == NULL);
	EXPECT_TRUE(japi_pushsrv_register(ctx,"") == NULL);

	/* Clean up */
	japi_destroy(ctx);
}

TEST(JAPI_Push_Service,SubscribeAndUnsubscribe)
{
	int socket;
	char* pushsrv_name;
	bool bval;
	japi_context *ctx;
	json_object *jreq;
	json_object *jresp;
	json_object *illegal_req;
	json_object *bad_req;

	socket = 4;
	pushsrv_name = (char*)"test_pushsrv";
	jreq = json_object_new_object();
	jresp = json_object_new_object();
	illegal_req = json_object_new_object();
	bad_req = json_object_new_object();
	ctx = japi_init(NULL);

	/* Build JSON request */
	json_object_object_add(jreq,"service",json_object_new_string(pushsrv_name));

	/* Sub-/unsubscribe before registering, expecting false */
	japi_pushsrv_subscribe(ctx,socket,jreq,jresp);
	EXPECT_EQ(japi_get_value_as_bool(jresp, "success",&bval),0);
	EXPECT_FALSE(bval);

	japi_pushsrv_unsubscribe(ctx,socket,jreq,jresp);
	EXPECT_EQ(japi_get_value_as_bool(jresp, "success",&bval),0);
	EXPECT_FALSE(bval);

	/* Pass illegal JSON request, expecting false */
	json_object_object_add(illegal_req,"service",NULL);

	japi_pushsrv_subscribe(ctx,socket,illegal_req,jresp);
	EXPECT_EQ(japi_get_value_as_bool(jresp, "success",&bval),0);
	EXPECT_FALSE(bval);

	japi_pushsrv_unsubscribe(ctx,socket,illegal_req,jresp);
	EXPECT_EQ(japi_get_value_as_bool(jresp, "success",&bval),0);
	EXPECT_FALSE(bval);

	/* Pass illegal key */
	json_object_object_add(bad_req,"bad_key",json_object_new_string(pushsrv_name));

	japi_pushsrv_subscribe(ctx,socket,bad_req,jresp);
	EXPECT_EQ(japi_get_value_as_bool(jresp, "success",&bval),0);
	EXPECT_FALSE(bval);

	japi_pushsrv_unsubscribe(ctx,socket,bad_req,jresp);
	EXPECT_EQ(japi_get_value_as_bool(jresp, "success",&bval),0);
	EXPECT_FALSE(bval);

	/* Try to unsubscribe without subscribed before, should fail */
	japi_pushsrv_register(ctx,"test_pushsrv");
	japi_pushsrv_unsubscribe(ctx,socket,jreq,jresp);
	EXPECT_EQ(japi_get_value_as_bool(jresp, "success",&bval),0);
	EXPECT_FALSE(bval);

	/* Expect true */
	japi_pushsrv_subscribe(ctx,socket,jreq,jresp);
	EXPECT_EQ(japi_get_value_as_bool(jresp, "success",&bval),0);
	EXPECT_TRUE(bval);

	japi_pushsrv_unsubscribe(ctx,socket,jreq,jresp);
	EXPECT_EQ(japi_get_value_as_bool(jresp, "success",&bval),0);
	EXPECT_TRUE(bval);

	/* Clean up */
	json_object_put(jresp);

}

TEST(JAPI_Push_Service,List)
{
	japi_context *ctx;
	japi_pushsrv_context *psc;
	json_object *jobj;

	ctx = japi_init(NULL);
	jobj = json_object_new_object();

	/* Register some test services */
	japi_pushsrv_register(ctx,"test01");
	japi_pushsrv_register(ctx,"test02");
	japi_pushsrv_register(ctx,"test03");

	/* The function to be tested */
	japi_pushsrv_list(ctx, NULL, jobj);

	psc = ctx->push_services;

	/* Iterate push service array & context and compare strings */
	json_object_object_foreach(jobj, key, val) {
		json_object_object_get_ex(jobj, key, &val);
		int arraylen = json_object_array_length(val);
		int i;
		json_object * jvalue;
		while (psc != NULL) {
			for (i=0; i< arraylen; i++) {
				jvalue = json_object_array_get_idx(val, i);
				EXPECT_STREQ(json_object_get_string(jvalue),psc->pushsrv_name);
				psc = psc->next;
			}
		}
	}

	/* Clean up */
	japi_destroy(ctx);
	json_object_put(jobj);
}

TEST(JAPI,AddRemoveClient)
{
	japi_context *ctx;
	japi_client *client;
	int counter;

	ctx = japi_init(NULL);

	/* Add some clients */
	EXPECT_EQ(japi_add_client(ctx,4),0);
	EXPECT_EQ(japi_add_client(ctx,5),0);
	EXPECT_EQ(japi_add_client(ctx,6),0);
	EXPECT_EQ(japi_add_client(ctx,7),0);

	/* Add the same client again */
	EXPECT_EQ(japi_add_client(ctx,5),0);
	EXPECT_EQ(japi_add_client(ctx,5),0);

	counter = 0;
	client = ctx->clients;
	while (client != NULL) {
		counter++;
		client = client->next;
	}
	/* Counter should count 6 added clients */
	EXPECT_EQ(counter,6);

	/* Remove some clients */
	EXPECT_EQ(japi_remove_client(ctx,4),0);
	EXPECT_EQ(japi_remove_client(ctx,5),0);

	counter = 0;
	client = ctx->clients;
	while (client != NULL) {
		counter++;
		client = client->next;
	}
	/* Counter should count 2 less clients */
	EXPECT_EQ(counter,4);

	/* Remove not existent client */
	EXPECT_EQ(japi_remove_client(ctx,12),-1);
	EXPECT_EQ(japi_remove_client(ctx,13),-1);
}

TEST(JAPI_Push_Service,AddRemoveClient)
{
	japi_context *ctx;
	japi_pushsrv_context *psc;
	japi_client *client;
	json_object *jobj;
	json_object *push_status_jreq;
	json_object *push_temperature_jreq;
	int counter;
	bool bval;

	push_status_jreq = json_object_new_object();
	push_temperature_jreq = json_object_new_object();
	jobj = json_object_new_object();
	ctx = japi_init(NULL);

	/* Build JSON request */
	json_object_object_add(push_status_jreq,"service",json_object_new_string("pushsrv_status"));
	json_object_object_add(push_temperature_jreq,"service",json_object_new_string("pushsrv_temperature"));

	/* Register some push services */
	japi_pushsrv_register(ctx,"pushsrv_status");
	japi_pushsrv_register(ctx,"pushsrv_temperature");

	/* Add some clients */
	japi_pushsrv_subscribe(ctx,4,push_temperature_jreq,jobj);
	japi_pushsrv_subscribe(ctx,5,push_temperature_jreq,jobj);
	japi_pushsrv_subscribe(ctx,6,push_temperature_jreq,jobj);
	/* Add a clients a second time */
	japi_pushsrv_subscribe(ctx,7,push_temperature_jreq,jobj);
	japi_pushsrv_subscribe(ctx,7,push_temperature_jreq,jobj);

	japi_pushsrv_subscribe(ctx,5,push_status_jreq,jobj);

	counter = 0;
	psc = ctx->push_services;
	client = psc->clients;
	while (client != NULL) {
		counter++;
		client = client->next;
	}
	/* Counter should count 5 clients for get_temperature */
	EXPECT_EQ(counter,5);

	/* Unsubscribe some clients */
	japi_pushsrv_unsubscribe(ctx,5,push_temperature_jreq,jobj);
	japi_pushsrv_unsubscribe(ctx,6,push_temperature_jreq,jobj);

	counter = 0;
	psc = ctx->push_services;
	client = psc->clients;
	while (client != NULL) {
		counter++;
		client = client->next;
	}
	/* Two less clients should be counted for get_temperature */
	EXPECT_EQ(counter,3);

	/* Unsubscribe client that is not subscribed */
	japi_pushsrv_unsubscribe(ctx,15,push_temperature_jreq,jobj);
	EXPECT_EQ(japi_get_value_as_bool(jobj, "success",&bval),0);
	EXPECT_FALSE(bval);
}

TEST(JAPI_Push_Service,PushServiceDestroy)
{
	japi_context *ctx;
	japi_pushsrv_context *psc_status, *psc_temperature;

	ctx = japi_init(NULL);

	/* Register some push services */
	psc_temperature = japi_pushsrv_register(ctx,"pushsrv_status");
	psc_status = japi_pushsrv_register(ctx,"pushsrv_temperature");

	/* Destroy push services */
	EXPECT_EQ(japi_pushsrv_destroy(psc_status),0);
	EXPECT_EQ(japi_pushsrv_destroy(psc_temperature),0);

	/* Pass bad push service context */
	EXPECT_EQ(japi_pushsrv_destroy(NULL),-1);
}
